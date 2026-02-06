#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_BCI_BATTERY
#include <linux/power/hisi/bci_battery.h>
#endif
#include <chipset_common/hwpower/wireless_charge/wireless_trx_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <idtp9221.h>
#include <idtp9221_fw_sram.h>

#define HWLOG_TAG wireless_idtp9221
HWLOG_REGIST();

static struct qi_protocol_handle *g_idtp9221_handle;
static struct idtp9221_device_info *g_idtp9221_di;
static struct wakeup_source g_idtp9221_wakelock;
static int g_support_idt_wlc;
static int g_support_st_wlc;
static int g_i2c_fail_cnt;
static bool g_st33_need_rst_flag;
static int stop_charging_flag;
static u16 stwlc33_iout_adc_val;
static int stwlc33_rx_ready_check_flag;
static int irq_abnormal_flag = false;
static u8 st_nvm_bufs[STWLC33_NVM_SEC_VAL_SIZE];
static const u8 idtp9221_send_msg_len[IDT9221_RX_TO_TX_DATA_LEN + 2] = { 0, 0x18, 0x28, 0x38, 0x48, 0x58 };
static const u8 idtp9221_send_fsk_msg_len[IDT9221_RX_TO_TX_DATA_LEN + 2] = { 0, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f };
static int g_chip_id;
static int g_rx_vmax;

static int idtp9221_read_block(struct idtp9221_device_info *di, u16 reg, u8 *data, u8 len)
{
	struct i2c_msg msg[2];
	int ret;
	int i;
	u8 reg_be[IDT9221_ADDR_LEN];

	reg_be[0] = reg >> BITS_PER_BYTE;
	reg_be[1] = reg & BYTE_MASK;

	if (!di->client->adapter)
		return -ENODEV;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = reg_be;
	msg[0].len = IDT9221_ADDR_LEN;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = data;
	msg[1].len = len;

	if ((di->i2c_trans_fail_limit > 0) &&
		(g_i2c_fail_cnt >= di->i2c_trans_fail_limit) &&
		(reg != IDT9221_SYS_MODE_ADDR) &&
		(reg != IDT9221_RX_INT_STATUS_ADDR) &&
		(reg != IDT9221_TX_VIN_ADDR)) {
		hwlog_err("%s: i2c err cnt = %d\n",
			__func__, g_i2c_fail_cnt);
		return -1;
	}
	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(di->client->adapter, msg, 2);
		if (ret >= 0)
			break;
		++g_i2c_fail_cnt;
		usleep_range(9500, 10500); /* 10ms */
	}

	if (ret < 0) {
		hwlog_err("idt9221 read block fail, start_reg = 0x%x\n", reg);
		return -1;
	}
	g_i2c_fail_cnt = 0;

	return 0;
}

static int idtp9221_write_block(struct idtp9221_device_info *di, u16 reg, u8 *buff, u8 len)
{
	struct i2c_msg msg;
	int ret;
	int i;

	if (!di->client->adapter)
		return -ENODEV;

	buff[0] = reg >> BITS_PER_BYTE;
	buff[1] = reg & BYTE_MASK;

	msg.buf = buff;
	msg.addr = di->client->addr;
	msg.flags = 0;
	msg.len = len + IDT9221_ADDR_LEN;

	if ((di->i2c_trans_fail_limit > 0) &&
		(g_i2c_fail_cnt >= di->i2c_trans_fail_limit) &&
		(reg != IDT9221_RX_INT_STATUS_ADDR)) {
		hwlog_err("%s: i2c err cnt = %d\n",
			__func__, g_i2c_fail_cnt);
		return -1;
	}
	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(di->client->adapter, &msg, 1);
		if (ret >= 0)
			break;
		++g_i2c_fail_cnt;
		usleep_range(9500, 10500); /* 10ms */
	}

	if (ret != 1) {
		hwlog_err("idt9221 write block fail, start_reg = 0x%x\n", reg);
		return -1;
	}
	g_i2c_fail_cnt = 0;

	return 0;
}

static int idtp9221_read_byte(u16 reg, u8 *data)
{
	struct idtp9221_device_info *di = g_idtp9221_di;

	return idtp9221_read_block(di, reg, data, BYTE_LEN);
}

static int idtp9221_read_word(u16 reg, u16 *data)
{
	struct idtp9221_device_info *di = g_idtp9221_di;
	u8 buff[WORD_LEN] = { 0 };
	int ret;

	if (!di || !data)
		return -EINVAL;

	ret = idtp9221_read_block(di, reg, buff, WORD_LEN);
	if (ret)
		return -1;

	*data = buff[0] | buff[1] << BITS_PER_BYTE;
	return 0;
}

static int idtp9221_write_byte(u16 reg, u8 data)
{
	struct idtp9221_device_info *di = g_idtp9221_di;
	u8 buff[IDT9221_ADDR_LEN + BYTE_LEN];

	buff[IDT9221_ADDR_LEN] = data;

	return idtp9221_write_block(di, reg, buff, BYTE_LEN);
}

static int idtp9221_write_word(u16 reg, u16 data)
{
	struct idtp9221_device_info *di = g_idtp9221_di;
	u8 buff[IDT9221_ADDR_LEN + WORD_LEN];

	buff[IDT9221_ADDR_LEN] = data  & BYTE_MASK;
	buff[IDT9221_ADDR_LEN + 1] = data >> BITS_PER_BYTE;

	return idtp9221_write_block(di, reg, buff, 2);
}

static int idtp9221_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = idtp9221_read_word(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	ret = idtp9221_write_word(reg, val);

	return ret;
}

static void idtp9221_wake_lock(void)
{
	if (!g_idtp9221_wakelock.active) {
		__pm_stay_awake(&g_idtp9221_wakelock);
		hwlog_info("idtp9221 wake lock\n");
	}
}

static void idtp9221_wake_unlock(void)
{
	if (g_idtp9221_wakelock.active) {
		__pm_relax(&g_idtp9221_wakelock);
		hwlog_info("idtp9221 wake unlock\n");
	}
}

static void idtp9221_enable_irq(struct idtp9221_device_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (di->irq_active == 0) {
		enable_irq(di->irq_int);
		di->irq_active = 1;
	}
	mutex_unlock(&di->mutex_irq);
}

static void idtp9221_disable_irq_nosync(struct idtp9221_device_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (di->irq_active == 1) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = 0;
	}
	mutex_unlock(&di->mutex_irq);
}

static int idtp9221_get_chip_id(u16 *chip_id)
{
	return idtp9221_read_word(IDT9221_RX_CHIP_ID_ADDR, chip_id);
}

static int idtp9221_clear_interrupt(u16 itr)
{
	int ret;
	u8 itrs[IDT9221_ADDR_LEN + IDT9221_RX_INT_CLEAR_LEN] = { 0 };
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s para is null\n", __func__);
		return -1;
	}

	/* clear interrupt set 1 */
	itrs[IDT9221_ADDR_LEN] = itrs[IDT9221_ADDR_LEN] | (itr & BYTE_MASK);
	itrs[IDT9221_ADDR_LEN + 1] = itrs[IDT9221_ADDR_LEN + 1] | (itr >> BITS_PER_BYTE);

	ret = idtp9221_write_block(di, IDT9221_RX_INT_CLEAR_ADDR,
		itrs, IDT9221_RX_INT_CLEAR_LEN);
	if (ret) {
		hwlog_err("%s:write interrupt clear register failed\n", __func__);
		return -1;
	}
	ret = idtp9221_write_byte(IDT9221_CMD_ADDR, IDT9221_CMD_CLEAR_INTERRUPT);
	if (ret) {
		hwlog_err("%s:write interrupt clear cmd failed\n", __func__);
		return -1;
	}
	return 0;
}

static int idtp9221_send_msg(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	u8 msg_len;
	u8 write_data[IDT9221_RX_TO_TX_DATA_LEN + IDT9221_ADDR_LEN] = { 0 };
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s para is null\n", __func__);
		return -1;
	}

	if ((data_len > IDT9221_RX_TO_TX_DATA_LEN) || (data_len < 0)) {
		hwlog_err("%s:send data out of range\n", __func__);
		return -1;
	}

	di->irq_val &= ~IDT9221_RX_STATUS_TX2RX_ACK;
	msg_len = idtp9221_send_msg_len[data_len + 1];

	ret = idtp9221_write_byte(IDT9221_RX_TO_TX_HEADER_ADDR, msg_len);
	if (ret) {
		hwlog_err("%s:write header failed\n", __func__);
		return -1;
	}
	ret = idtp9221_write_byte(IDT9221_RX_TO_TX_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s:write cmd failed\n", __func__);
		return -1;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data + IDT9221_ADDR_LEN, data, data_len);
		ret = idtp9221_write_block(di, IDT9221_RX_TO_TX_DATA_ADDR, write_data, data_len);
		if (ret) {
			hwlog_err("%s:write data into RX to TX register failed\n", __func__);
			return -1;
		}
	}

	ret = idtp9221_write_byte(IDT9221_CMD_ADDR, IDT9221_CMD_SEND_RX_DATA);
	if (ret) {
		hwlog_err("%s:send msg to tx failed\n", __func__);
		return -1;
	}
	hwlog_info("%s:send msg success\n", __func__);
	return 0;
}

static int idtp9221_send_msg_ack(u8 cmd, u8 *data, int data_len,
	void *dev_data)
{
	int count = 0;
	int ack_cnt;
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s para is null\n", __func__);
		return -1;
	}

	do {
		idtp9221_send_msg(cmd, data, data_len, di);
		for (ack_cnt = 0; ack_cnt < IDT9221_WAIT_FOR_ACK_RETRY_CNT; ack_cnt++) {
			msleep(IDT9221_WAIT_FOR_ACK_SLEEP_TIME);
			if (IDT9221_RX_STATUS_TX2RX_ACK & di->irq_val) {
				di->irq_val &= ~IDT9221_RX_STATUS_TX2RX_ACK;
				hwlog_info("[%s] succ, retry times = %d\n", __func__, count);
				return 0;
			}
			if (stop_charging_flag) {
				hwlog_err("%s: already stop charging, return\n", __func__);
				return -1;
			}
		}
		count++;
		hwlog_info("[%s] retry\n", __func__);
	} while (count < IDT9221_SNED_MSG_RETRY_CNT);

	if (count == IDT9221_SNED_MSG_RETRY_CNT) {
		hwlog_err("[%s] fail, retry times = %d\n", __func__, count);
		return -1;
	}
	return 0;
}

static int idtp9221_receive_msg(u8 *data, int data_len, void *dev_data)
{
	int ret;
	int count = 0;
	u8 buff[IDT9221_TX_TO_RX_MESSAGE_LEN] = {0};
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di || !data) {
		hwlog_err("%s para is null\n", __func__);
		return -1;
	}

	(void)idtp9221_write_block(di, IDT9221_TX_TO_RX_CMD_ADDR, buff,
		IDT9221_TX_TO_RX_MESSAGE_LEN);

	do {
		(void)idtp9221_read_block(di, IDT9221_TX_TO_RX_CMD_ADDR, data, data_len);
		if (di->irq_val & IDT9221_RX_STATUS_TXDATA_RECEIVED) {
			di->irq_val &= ~IDT9221_RX_STATUS_TXDATA_RECEIVED;
			goto FuncEnd;
		}
		if (stop_charging_flag) {
			hwlog_err("%s already stop charging, return\n", __func__);
			return -1;
		}
		msleep(IDT9221_RCV_MSG_SLEEP_TIME);
		count++;
	} while (count < IDT9221_RCV_MSG_SLEEP_CNT);

FuncEnd:
	ret = idtp9221_read_block(di, IDT9221_TX_TO_RX_CMD_ADDR, data, data_len);
	if (ret) {
		hwlog_err("%s:get tx to rx data failed\n", __func__);
		return -1;
	}
	if (!data[0]) {
		hwlog_err("%s: no msg received from tx\n", __func__);
		return -1;
	}
	hwlog_info("[%s] get tx to rx data, succ\n", __func__);
	return 0;
}

static void idtp9221_chip_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct idtp9221_device_info *di = dev_data;

	if (!di)
		return;

	gpio_set_value(di->gpio_en,
		enable ? di->gpio_en_valid_val : !di->gpio_en_valid_val);
	gpio_val = gpio_get_value(di->gpio_en);
	hwlog_info("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
}

static bool idtp9221_is_chip_enable(void *dev_data)
{
	int gpio_val;
	struct idtp9221_device_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_en);
	return gpio_val == di->gpio_en_valid_val;
}

static void idtp9221_sleep_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct idtp9221_device_info *di = dev_data;

	if (!di || irq_abnormal_flag)
		return;

	gpio_set_value(di->gpio_sleep_en,
		enable ? WLTRX_IC_SLEEP_ENABLE : WLTRX_IC_SLEEP_DISABLE);
	gpio_val = gpio_get_value(di->gpio_sleep_en);
	hwlog_info("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
}

static bool idtp9221_is_sleep_enable(void *dev_data)
{
	int gpio_val;
	struct idtp9221_device_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_sleep_en);
	return gpio_val == WLTRX_IC_SLEEP_ENABLE;
}

static void idtp9221_chip_reset(void *dev_data)
{
	int ret;
	u16 chip_id = 0;

	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return;

	if (chip_id == STWLC33_CHIP_ID)
		ret = idtp9221_write_byte(STWLC33_CHIP_RST_ADDR,
			STWLC33_CHIP_RST_VAL);
	else
		ret = idtp9221_write_byte(IDT9221_M0_ADDR,
			IDT9221_M0_RESET);
	if (ret)
		hwlog_err("%s fail\n", __func__);
}

static void idtp9221_send_ept(int ept_type, void *dev_data)
{
	int ret;
	u8 rx_ept_type;

	switch (ept_type) {
	case WIRELESS_EPT_ERR_VRECT:
		rx_ept_type = IDT9221_RX_ERR_VRECT;
		break;
	case WIRELESS_EPT_ERR_VOUT:
		rx_ept_type = IDT9221_RX_ERR_VOUT;
		break;
	default:
		hwlog_err("send_ept: 0x%x error\n", ept_type);
		return;
	}
	ret = idtp9221_write_byte(IDT9221_RX_EPT_ADDR, rx_ept_type);
	ret += idtp9221_write_byte(IDT9221_CMD_ADDR, IDT9221_CMD_SEND_EPT);
	if (ret)
		hwlog_err("send_ept: failed\n");
}

static int idtp9221_get_rx_vrect(int *vrect, void *dev_data)
{
	int ret;
	u8 volt[IDT9221_RX_VRECT_LEN];
	u32 vol;
	struct idtp9221_device_info *di = dev_data;

	if (!di || !vrect) {
		hwlog_err("%s di null\n", __func__);
		return -1;
	}
	ret = idtp9221_read_block(di, IDT9221_RX_GET_VRECT_ADDR, volt, IDT9221_RX_VRECT_LEN);
	if (ret) {
		hwlog_err("get idt9221 rx vrect failed\n");
		return -1;
	}
	vol = volt[1];
	vol = (vol << BITS_PER_BYTE) + volt[0];
	*vrect = (vol * IDT9221_RX_VRECT_VALUE_MAX / IDT9221_RX_VRECT_REG_MAX);

	return 0;
}

static int idtp9221_get_rx_vout(int *vout, void *dev_data)
{
	int ret;
	u8 volt[IDT9221_RX_VOUT_LEN];
	u32 vol;
	struct idtp9221_device_info *di = dev_data;

	if (!di || !vout) {
		hwlog_err("%s di null\n", __func__);
		return -1;
	}

	ret = idtp9221_read_block(di, IDT9221_RX_GET_VOUT_ADDR, volt, IDT9221_RX_VOUT_LEN);
	if (ret) {
		hwlog_err("get idt9221 rx vout failed\n");
		return -1;
	}

	vol = volt[1];
	vol = (vol << BITS_PER_BYTE) + volt[0];
	*vout = (vol * IDT9221_RX_VOUT_VALUE_MAX / IDT9221_RX_VOUT_REG_MAX);
	return 0;
}

static int idtp9221_get_rx_vout_reg(int *rx_vreg, void *dev_data)
{
	int ret;
	u8 vreg = 0;

	if (!rx_vreg)
		return -EINVAL;

	ret = idtp9221_read_byte(IDT9221_RX_SET_VOUT_ADDR, &vreg);
	if (ret) {
		hwlog_err("get idtp9221 rx vout reg failed\n");
		return -1;
	}

	*rx_vreg = (vreg + IDT9221_RX_VOUT_OFFSET / IDT9221_RX_VOUT_STEP) * IDT9221_RX_VOUT_STEP;
	return 0;
}

static int idtp9221_get_vfc_reg(int *vfc_reg, void *dev_data)
{
	return idtp9221_read_word(IDT9221_SET_TX_VOUT_ADDR, (u16 *)vfc_reg);
}

static int idtp9221_get_rx_iout(int *iout, void *dev_data)
{
	int ret;
	u8 curr[IDT9221_RX_IOUT_LEN];
	u32 cur;
	struct idtp9221_device_info *di = dev_data;

	if (!di || !iout) {
		hwlog_err("%s di null\n", __func__);
		return -1;
	}
	ret = idtp9221_read_block(di, IDT9221_RX_GET_IOUT_ADDR, curr, IDT9221_RX_IOUT_LEN);
	if (ret) {
		hwlog_err("get idt9221 rx iout value failed\n");
		return -1;
	}

	cur = curr[1];
	*iout = (cur << BITS_PER_BYTE) + curr[0];
	return 0;
}

static int idtp9221_set_rx_vout(int vol, void *dev_data)
{
	int ret;

	if ((vol < IDT9221_RX_VOUT_MIN) || (vol > IDT9221_RX_VOUT_MAX)) {
		hwlog_err("%s set ldo out voltage value out of range\n", __func__);
		return -1;
	}

	vol = vol / IDT9221_RX_VOUT_STEP - IDT9221_RX_VOUT_OFFSET / IDT9221_RX_VOUT_STEP;
	ret = idtp9221_write_byte(IDT9221_RX_SET_VOUT_ADDR, (u8)vol);
	if (ret) {
		hwlog_err("set idtp9221 rx vout failed\n");
		return -1;
	}

	return 0;
}

static void idtp9221_get_bst_delay_time(int *time, void *dev_data)
{
	if (!time)
		return;

	if (g_chip_id == STWLC33_CHIP_ID)
		*time = 1000; /* delay 1s after vout boost for stwlc33 */
	else
		*time = 3000; /* delay 3s after vout boost for idtp9221 */
}

static int idtp9221_set_vfc(int vfc, void *dev_data)
{
	int ret, i;
	int vout = 0;
	int cnt = IDT9221_SET_TX_VOUT_TIMEOUT / IDT9221_SET_TX_VOUT_SLEEP_TIME;
	u8 buff[IDT9221_ADDR_LEN + IDT9221_SET_TX_VOUT_LEN];
	struct idtp9221_device_info *di = dev_data;

	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return -1;
	}

	if ((g_rx_vmax > 0) && (vfc > g_rx_vmax)) {
		hwlog_err("rx abnormal, not support vout greater than %dmV", g_rx_vmax);
		return -1;
	}
	buff[IDT9221_ADDR_LEN] = vfc & BYTE_MASK;
	buff[IDT9221_ADDR_LEN + 1] = vfc >> BITS_PER_BYTE;

	ret = idtp9221_write_word(IDT9221_SET_TX_VOUT_ADDR, vfc);
	if (ret) {
		hwlog_err("%s: set tx vout reg failed\n", __func__);
		return -1;
	}
	ret = idtp9221_write_byte(IDT9221_CMD_ADDR, IDT9221_CMD_SEND_FAST_CHRG);
	if (ret) {
		hwlog_err("set tx vout cmd reg failed\n");
		return -1;
	}
	for (i = 0; i < cnt; i++) {
		msleep(IDT9221_SET_TX_VOUT_SLEEP_TIME);
		(void)idtp9221_get_rx_vout(&vout, dev_data);
		if (vout <= 0) {
			hwlog_err("%s: get rx vout failed\n", __func__);
			return -1;
		}
		if ((vout >= vfc - IDT9221_TX_VOUT_ERR_LTH) &&
			(vout <= vfc + IDT9221_TX_VOUT_ERR_UTH)) {
			return 0;
		}
		if (stop_charging_flag) {
			hwlog_err("%s already stop charging, return\n", __func__);
			return -1;
		}
	}
	return -1;
}

static int idtp9221_get_rx_fop(int *fop, void *dev_data)
{
	int ret;
	u8 val[IDT9221_RX_FOP_LEN] = {0};
	u32 l_fop;
	struct idtp9221_device_info *di = dev_data;

	if (!di || !fop) {
		hwlog_err("%s di null\n", __func__);
		return -1;
	}
	ret = idtp9221_read_block(di, IDT9221_RX_GET_FOP_ADDR, val, IDT9221_RX_FOP_LEN);
	if (ret) {
		hwlog_err("get idt9221 rx fop failed\n");
		return -1;
	}
	l_fop = val[1];
	l_fop = (l_fop << BITS_PER_BYTE) + val[0];
	if (l_fop)
		*fop = IDT9221_RX_FOP_COEF / l_fop;

	return 0;
}

static char *stwlc33_nvm_read(int sec_no)
{
	int i;
	int ret;
	u16 cmd_status = 0;
	u8 nvm_sec_val[STWLC33_NVM_REG_SIZE] = { 0 };
	static char nvm_buf[STWLC33_NVM_VALUE_SIZE];
	char temp[STWLC33_NVM_SEC_VAL_SIZE] = { 0 };
	u8 sec_val[STWLC33_SEC_NO_SIZE] = { 0 };
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return "error";
	}
	if ((sec_no < 0) || (sec_no > STWLC33_NVM_SEC_NO_MAX)) {
		hwlog_err("%s: nvm sec no is invalid\n", __func__);
		return "error";
	}
	memset(nvm_buf, 0, STWLC33_NVM_VALUE_SIZE);
	/* Step 1 : check for zero, to confirm pre cmds completed */
	ret = idtp9221_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
	if (ret) {
		hwlog_err("%s: get cmd status failed\n", __func__);
		return "error";
	}
	if (cmd_status) {
		hwlog_err("%s: pre cmds not completed [0x%x]\n",
			__func__, cmd_status);
		return "error";
	}
	/* Step 2 : Update I2C sector no */
	sec_val[0] = sec_no;
	memcpy(nvm_sec_val + STWLC33_ADDR_LEN, sec_val, STWLC33_SEC_NO_SIZE);
	ret = idtp9221_write_block(di, STWLC33_OFFSET_REG_ADDR, nvm_sec_val,
		STWLC33_SEC_NO_SIZE);
	if (ret) {
		hwlog_err("%s: write sector no fail\n", __func__);
		return "error";
	}
	/* Step 3 : Update I2C sector value length */
	ret = idtp9221_write_byte(STWLC33_SRAM_SIZE_ADDR,
		STWLC33_NVM_SEC_VAL_SIZE - 1);
	if (ret) {
		hwlog_err("%s: write sector length fail\n", __func__);
		return "error";
	}
	/* Step 4 : Write I2C nvm read cmd val */
	ret = idtp9221_write_byte(STWLC33_CMD_STATUS_ADDR, STWLC33_NVM_RD_CMD);
	if (ret) {
		hwlog_err("%s: write nvm read cmd fail\n", __func__);
		return "error";
	}
	/* Step 5 : check for zero, to confirm pre cmds completed */
	ret = idtp9221_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
	if (ret) {
		hwlog_err("%s: get cmd status failed\n", __func__);
		return "error";
	}
	if (cmd_status) {
		hwlog_err("%s: pre cmds not completed [0x%x]\n",
			__func__, cmd_status);
		return "error";
	}
	/* Step 6 : Read back corresponding NVM sector content */
	ret = idtp9221_read_block(di, STWLC33_NVM_RD_ADDR,
		st_nvm_bufs, STWLC33_NVM_SEC_VAL_SIZE);
	if (ret) {
		hwlog_err("%s: read NVM data failed\n", __func__);
		return "error";
	}
	for (i = 0; i < STWLC33_NVM_SEC_VAL_SIZE; i++) {
		snprintf(temp, STWLC33_NVM_SEC_VAL_SIZE, "%x ", st_nvm_bufs[i]);
		strncat(nvm_buf, temp, strlen(temp));
	}

	return nvm_buf;
}

static char *idt_st_read_nvm(int sec_num, void *dev_data)
{
	int ret;
	u16 chip_id = 0;

	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return "i2c error";

	if (chip_id == STWLC33_CHIP_ID)
		return stwlc33_nvm_read(sec_num);

	return "not support yet";
}

static int idtp9221_get_mode(u8 *mode)
{
	int ret;

	ret = idtp9221_read_byte(IDT9221_SYS_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("%s:read failed\n", __func__);
		return -1;
	}
	return 0;
}

static bool idtp9221_is_tx_exist(void *dev_data)
{
	int ret;
	u8 mode = 0;
	int charger_vbus;

	ret = idtp9221_get_mode(&mode);
	if (ret) {
		hwlog_err("%s: get rx mode failed\n", __func__);
		goto mode_err;
	}

	if ((mode & IDT9221_WPC_MODE) || (mode & IDT9221_PMA_MODE))
		return true;

mode_err:
	if (g_chip_id == STWLC33_CHIP_ID) {
		msleep(50); /* delay 50ms for vbus stability */
		charger_vbus = get_charger_vbus_vol();
		if (charger_vbus >= RX_HIGH_VOUT)
			g_rx_vmax = 9000; /* vout greater than 9V is forbidden */
	}

	return false;
}

static int idtp9221_kick_watchdog(void *dev_data)
{
	int ret;

	ret = idtp9221_write_word(IDT9221_RX_FC_TIMER_ADDR, 0);
	if (ret) {
		hwlog_err("%s: write register[0x%x] failed\n", __func__, IDT9221_RX_FC_TIMER_ADDR);
		return -1;
	}
	return 0;
}

static int idtp9221_program_sramupdate(const struct fw_update_info *fw_sram_info)
{
	u8 mode;
	u8 bufs[IDT9221_PAGE_SIZE + IDT9221_ADDR_LEN]; /* write datas must offset 2 bytes */
	u8 write_size;
	u16 offset;
	int i, j;
	int ret;
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di || !fw_sram_info || !fw_sram_info->fw_sram) {
		hwlog_err("%s:para is null\n", __func__);
		return -1;
	}

	/* === Step -1 : check idtp9221 mode === */
	ret = idtp9221_get_mode(&mode);
	if (ret) {
		hwlog_err("%s:get mode failed\n", __func__);
		return -1;
	}
	if (!(IDT9221_OTPONLY_MODE & mode)) {
		hwlog_err("%s:not runing in OTP\n", __func__);
		return -1;
	}

	/* === Step -2 : write FW into SRAM and check === */
	i = fw_sram_info->fw_sram_size;
	offset = 0;
	while (i > 0) {
		if (i >= IDT9221_PAGE_SIZE)
			write_size = IDT9221_PAGE_SIZE;
		else
			write_size = i;
		memcpy(bufs + IDT9221_ADDR_LEN, fw_sram_info->fw_sram + offset, write_size);
		ret = idtp9221_write_block(di, fw_sram_info->fw_sram_update_addr + offset,
			bufs, write_size);
		if (ret) {
			hwlog_err("%s:write SRAM firmware failed\n", __func__);
			return -1;
		}
		offset += write_size;
		i -= write_size;
	}

	i = fw_sram_info->fw_sram_size;
	offset = 0;
	while (i > 0) {
		if (i >= IDT9221_PAGE_SIZE)
			write_size = IDT9221_PAGE_SIZE;
		else
			write_size = i;
		ret = idtp9221_read_block(di, fw_sram_info->fw_sram_update_addr + offset,
			bufs, write_size);
		if (ret) {
			hwlog_err("%s: read SRAM failed\n", __func__);
			return -1;
		}
		for (j = 0; j < write_size; j++) {
			if (bufs[j] != fw_sram_info->fw_sram[offset + j]) {
				hwlog_err("%s:not equal: read = 0x%x, write = 0x%x\n", __func__, bufs[j], fw_sram_info->fw_sram[offset + j]);
				return -1;
			}
		}
		i -= write_size;
		offset += write_size;
	}

	/*  === Step -3 : switch to SRAM running === */
	if (fw_sram_info->fw_sram_mode == WIRELESS_TX_MODE) {
		ret = idtp9221_write_byte(IDT9221_KEY_ADDR, IDT9221_KEY_VALUE);
		ret |= idtp9221_write_byte(IDT9221_M0_ADDR, IDT9221_M0_UNDO);
		ret |= idtp9221_write_byte(IDT9221_MAP_ADDR, IDT9221_MAP_OTP2RAM);
		if (ret) {
			hwlog_err("%s: write 0x3000/0x3040/0x3048 failed\n", __func__);
			return -1;
		}
		/* ignoreNAK */
		idtp9221_write_byte(IDT9221_M0_ADDR, IDT9221_M0_RESET); /* reset chip and run the bootloader.if noACK writing error */
	}
	ret = idtp9221_write_byte(IDT9221_CMD1_ADDR, IDT9221_CMD1_UNLOCK_SWITCH); /* unlock the switch */
	ret |= idtp9221_write_byte(IDT9221_CMD_ADDR, IDT9221_CMD_SWITCH_TO_SRAM); /* switch to SRAM */
	if (ret) {
		hwlog_err("%s: write 0x004e/0x004f failed\n", __func__);
		return -1;
	}
	msleep(50); /* wait for swtiching to SRAM running */
	ret = idtp9221_get_mode(&mode);
	if (!(IDT9221_RAMPROGRAM_MODE & mode)) {
		hwlog_err("%s: switch to SRAM running failed\n", __func__);
		return -1;
	}
	if (fw_sram_info->fw_sram_mode == WIRELESS_TX_MODE) {
		ret = idtp9221_write_word(IDT9221_TX_EPT_TYPE_ADDR, 0);
		if (ret) {
			hwlog_err("%s: write failed\n", __func__);
			return -1;
		}
	}
	return 0;
}

static int stwlc33_write_ram_data(const struct fw_update_info *fw_sram_info)
{
	int i;
	int ret;
	u16 offset;
	u16 cmd_status;
	u8 write_size;
	/* write datas must offset 2 bytes */
	u8 bufs[STWLC33_PAGE_SIZE + STWLC33_ADDR_LEN];
	u8 offset_reg_value[STWLC33_OFFSET_REG_SIZE] = { 0 };
	u8 st_fw_start_addr[] = { 0x00, 0x00, 0x00, 0x00 };
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di || !fw_sram_info || !fw_sram_info->fw_sram) {
		hwlog_err("%s: para is null\n", __func__);
		return -ENODEV;
	}

	/* write FW into SRAM */
	i = fw_sram_info->fw_sram_size;
	offset = 0;
	while (i > 0) {
		if (i >= STWLC33_PAGE_SIZE)
			write_size = STWLC33_PAGE_SIZE;
		else
			write_size = i;
		/* Step 1 : confirm previous commands completed  */
		ret = idtp9221_read_word(STWLC33_CMD_STATUS_ADDR,
			&cmd_status);
		if (ret) {
			hwlog_err("%s: get cmd status fail\n", __func__);
			return -EINVAL;
		}
		if (cmd_status) {
			hwlog_err("%s: pre cmd not completed [0x%x]\n",
				__func__, cmd_status);
			return -EINVAL;
		}
		/* Step 2 : Load data */
		memcpy(bufs + STWLC33_ADDR_LEN, fw_sram_info->fw_sram + offset,
			write_size);
		ret = idtp9221_write_block(di,
			fw_sram_info->fw_sram_update_addr, bufs, write_size);
		if (ret) {
			hwlog_err("%s: write SRAM fm failed\n", __func__);
			return -EINVAL;
		}
		/* Step 3 : Update I2C reg mem_access_offset */
		st_fw_start_addr[0] = offset & BYTE_MASK;
		st_fw_start_addr[1] = offset >> BITS_PER_BYTE;
		memcpy(offset_reg_value + STWLC33_ADDR_LEN, st_fw_start_addr,
			STWLC33_OFFSET_VALUE_SIZE);
		ret = idtp9221_write_block(di, STWLC33_OFFSET_REG_ADDR,
			offset_reg_value, STWLC33_OFFSET_VALUE_SIZE);
		/* Step 4 : Update number of bytes */
		ret |= idtp9221_write_byte(STWLC33_SRAM_SIZE_ADDR,
			write_size - 1);
		if (ret) {
			hwlog_err("%s: Update write size failed\n", __func__);
			return -EINVAL;
		}
		/* Step 5 : Issue patch_wr command */
		ret = idtp9221_write_byte(STWLC33_ACT_CMD_ADDR,
			STWLC33_WRITE_CMD_VALUE);
		if (ret) {
			hwlog_err("%s: Issue patch_wr cmd failed\n", __func__);
			return -EINVAL;
		}
		offset += write_size;
		i -= write_size;
	}

	hwlog_info("[%s] write FW into SRAM succ\n", __func__);
	return 0;
}

static int stwlc33_check_ram_data(const struct fw_update_info *fw_sram_info)
{
	int i;
	int j;
	int ret;
	u16 offset;
	u8 write_size;
	u16 cmd_status;
	/* write datas must offset 2 bytes */
	u8 bufs[STWLC33_PAGE_SIZE + STWLC33_ADDR_LEN];
	u8 offset_reg_value[STWLC33_OFFSET_REG_SIZE] = { 0 };
	u8 st_fw_start_addr[] = { 0x00, 0x00, 0x00, 0x00 };
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di || !fw_sram_info || !fw_sram_info->fw_sram) {
		hwlog_err("%s: para is null\n", __func__);
		return -EINVAL;
	}

	/* read ram patch and check */
	i = fw_sram_info->fw_sram_size;
	offset = 0;
	while (i > 0) {
		if (i >= STWLC33_PAGE_SIZE)
			write_size = STWLC33_PAGE_SIZE;
		else
			write_size = i;
		/* Step 1 : confirm previous commands completed */
		ret = idtp9221_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
		if (ret) {
			hwlog_err("%s: get cmd status failed\n", __func__);
			return -EINVAL;
		}
		if (cmd_status) {
			hwlog_err("%s: pre cmd not completed [0x%x]\n",
				__func__, cmd_status);
			return -EINVAL;
		}
		/* Step 2 : Update I2C reg mem_access_offset */
		st_fw_start_addr[0] = offset & BYTE_MASK;
		st_fw_start_addr[1] = offset >> BITS_PER_BYTE;
		memcpy(offset_reg_value + STWLC33_ADDR_LEN, st_fw_start_addr,
			STWLC33_OFFSET_VALUE_SIZE);
		ret = idtp9221_write_block(di, STWLC33_OFFSET_REG_ADDR,
			offset_reg_value, STWLC33_OFFSET_VALUE_SIZE);
		/* Step 3 : Update number of bytes */
		ret += idtp9221_write_byte(STWLC33_SRAM_SIZE_ADDR,
			write_size - 1);
		if (ret) {
			hwlog_err("%s: Update read size failed\n", __func__);
			return -EINVAL;
		}
		/* Step 4 : Issue patch_rd command */
		ret = idtp9221_write_byte(STWLC33_ACT_CMD_ADDR,
			STWLC33_READ_CMD_VALUE);
		if (ret) {
			hwlog_err("%s: Issue patch_rd cmd fail\n", __func__);
			return -EINVAL;
		}
		/* Step 5 : confirm previous commands completed */
		ret = idtp9221_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
		if (ret) {
			hwlog_err("%s: get cmds status failed\n", __func__);
			return -EINVAL;
		}
		if (cmd_status) {
			hwlog_err("%s: pre cmds not completed [0x%x]\n",
				__func__, cmd_status);
			return -EINVAL;
		}
		/* Step 6 : Read data from buffer and check */
		ret = idtp9221_read_block(di,
			fw_sram_info->fw_sram_update_addr, bufs, write_size);
		if (ret) {
			hwlog_err("%s: read SRAM failed\n", __func__);
			return -EINVAL;
		}
		for (j = 0; j < write_size; j++) {
			if (bufs[j] != fw_sram_info->fw_sram[offset + j]) {
				hwlog_err("%s: read (0x%x) != write (0x%x)\n",
					__func__, bufs[j],
					fw_sram_info->fw_sram[offset + j]);
				return -EINVAL;
			}
		}
		i -= write_size;
		offset += write_size;
	}

	hwlog_info("[%s] read and check ram patch succ\n", __func__);
	return 0;
}

static int stwlc33_exec_ram_data(void)
{
	int ret;
	u16 cmd_status;
	u8 offset_reg_val[STWLC33_OFFSET_REG_SIZE] = { 0 };
	unsigned char magic_number[] = { 0x01, 0x10, 0x00, 0x20 };
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s: para is null\n", __func__);
		return -EINVAL;
	}
	/* Step 1 : check for zero, to confirm previous commands completed */
	ret = idtp9221_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
	if (ret) {
		hwlog_err("%s: get cmd status failed\n", __func__);
		return -EINVAL;
	}
	if (cmd_status) {
		hwlog_err("%s: pre cmds not completed [0x%x]\n",
			__func__, cmd_status);
		return -EINVAL;
	}
	/* Step 2 : Write 4-byte magic number */
	memcpy(offset_reg_val + STWLC33_ADDR_LEN, magic_number,
		STWLC33_OFFSET_VALUE_SIZE);
	ret = idtp9221_write_block(di, STWLC33_OFFSET_REG_ADDR,
		offset_reg_val, STWLC33_OFFSET_VALUE_SIZE);
	if (ret) {
		hwlog_err("%s: Write magic number failed\n", __func__);
		return -EINVAL;
	}
	/* Step 3 : Issue patch_exec command */
	ret |= idtp9221_write_byte(STWLC33_ACT_CMD_ADDR,
		STWLC33_EXEC_CMD_VALUE);
	if (ret) {
		hwlog_err("%s: Issue patch_exec cmd failed\n", __func__);
		return -EINVAL;
	}
	/* wait for swtiching to SRAM running */
	msleep(STWLC33_SRAM_EXEC_TIME);
	/* Step 4 : check for zero, to confirm patch execution */
	ret = idtp9221_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
	if (ret) {
		hwlog_err("%s: get cmd_status failed\n", __func__);
		return -EINVAL;
	}
	if (cmd_status) {
		hwlog_err("%s: pre cmds not completed [0x%x]\n",
			__func__, cmd_status);
		return -EINVAL;
	}

	hwlog_info("[%s] exec ram patch succ\n", __func__);
	return 0;
}

static int stwlc33_program_sramupdate(const struct fw_update_info *fw_sram_info)
{
	int ret;
	u8 iout_offset;
	u16 ram_version;

	/* Step -1 write FW patch into SRAM */
	ret = stwlc33_write_ram_data(fw_sram_info);
	if (ret) {
		hwlog_err("%s: write ram data fail\n", __func__);
		return -EINVAL;
	}
	/* Step -2 read ram patch and check */
	ret = stwlc33_check_ram_data(fw_sram_info);
	if (ret) {
		hwlog_err("%s: check ram data fail\n", __func__);
		return -EINVAL;
	}
	/* Step -3 : To execute ram patch */
	ret = stwlc33_exec_ram_data();
	if (ret) {
		hwlog_err("%s: exec ram data fail\n", __func__);
		return -EINVAL;
	}
	if (fw_sram_info->fw_sram_mode == WIRELESS_TX_MODE) {
		usleep_range(9500, 10500); /* 10ms */
		idtp9221_write_byte(STWLC33_TX_ENABLE_ADDR,
			STWLC33_TX_ENABLE_VAL);
		ret = stwlc33_exec_ram_data();
		if (ret) {
			hwlog_err("%s: exec ram data fail\n", __func__);
			return -EINVAL;
		}
	}
	if ((fw_sram_info->fw_sram_mode == WIRELESS_RX_MODE) &&
		(stwlc33_iout_adc_val > STWLC33_IOUT_ADC_VAL_LTH) &&
		(stwlc33_iout_adc_val < STWLC33_IOUT_ADC_VAL_HTH)) {
		if (stwlc33_iout_adc_val >= STWLC33_IOUT_ADC_CRITICAL_VAL)
			iout_offset = stwlc33_iout_adc_val -
				STWLC33_IOUT_ADC_CRITICAL_VAL;
		else
			iout_offset = ~(STWLC33_IOUT_ADC_CRITICAL_VAL -
				stwlc33_iout_adc_val) + 1;
		hwlog_info("[%s] iout_offset = 0x%x\n", __func__, iout_offset);
		ret = idtp9221_write_byte(STWLC33_IOUT_CALI_VAL_ADDR,
			iout_offset);

		/* clear iout calibration register */
		ret += idtp9221_write_byte(STWLC33_IOUT_CALI_ADDR, 0x00);
		if (ret)
			hwlog_err("%s: write reg failed\n", __func__);
	}
	ret = idtp9221_read_word(STWLC33_RAM_VER_ADDR, &ram_version);
	if (ret) {
		hwlog_err("%s: read ram version fail\n", __func__);
		return -EINVAL;
	}
	hwlog_info("[%s] success ram_version:0x%x\n", __func__, ram_version);

	return 0;
}

static int idt_st_program_sramupdate(const struct fw_update_info *fw_sram_info)
{
	int ret;
	u16 chip_id = 0;

	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return ret;

	if (chip_id == STWLC33_CHIP_ID)
		return stwlc33_program_sramupdate(fw_sram_info);
	else if (chip_id == IDTP9221_CHIP_ID)
		return idtp9221_program_sramupdate(fw_sram_info);
	else
		return -EINVAL;
}

static int idtp9221_get_rx_fod_coef(char *coef_str, int len, void *dev_data)
{
	int ret, i;
	u16 fod_coef[IDT9221_RX_FOD_COEF_LEN] = { 0 };
	char tmp[IDT9221_RX_FOD_TMP_STR_LEN] = { 0 };

	if (!coef_str || (len < WLRX_IC_FOD_COEF_LEN))
		return -EINVAL;

	for (i = 0; i < IDT9221_RX_FOD_COEF_LEN; i++) {
		ret = idtp9221_read_word(IDT9221_RX_FOD_COEF_STSRT_ADDR +
			i * WORD_LEN, &fod_coef[i]);
		if (ret) {
			hwlog_err("%s: read fod_coed[%d] fail\n", __func__, i);
			return -EIO;
		}
	}
	for (i = 0; i < IDT9221_RX_FOD_COEF_LEN; i++) {
		snprintf(tmp, IDT9221_RX_FOD_TMP_STR_LEN, "%d ", fod_coef[i]);
		strncat(coef_str, tmp, strlen(tmp));
	}

	return strlen(coef_str);
}

static int idtp9221_set_rx_fod_coef(const char *fod_coef, void *dev_data)
{
	char *cur = NULL;
	char *token = NULL;
	u16 fod_arr[IDT9221_RX_FOD_COEF_LEN] = { 0 };
	int i = 0;
	long val = 0;
	int ret;

	if (!fod_coef) {
		hwlog_err("%s: input para err\n", __func__);
		return -1;
	}

	cur = (char *)fod_coef;
	token = strsep(&cur, " ,");
	while (token) {
		if ((strict_strtol(token, POWER_BASE_DEC, &val) < 0)) {
			hwlog_err("%s input para type err\n", __func__);
			return -1;
		}
		token = strsep(&cur, " ,");
		fod_arr[i++] = (u16)val;
		if (i == IDT9221_RX_FOD_COEF_LEN)
			break;
	}
	if (i < IDT9221_RX_FOD_COEF_LEN) {
		hwlog_err("%s input para number err\n", __func__);
		return -1;
	}
	for (i = 0; i < IDT9221_RX_FOD_COEF_LEN; i++) {
		hwlog_info("%s fod_coef[%d] = %u\n", __func__, i, fod_arr[i]);
		ret = idtp9221_write_word(IDT9221_RX_FOD_COEF_STSRT_ADDR + i * WORD_LEN, fod_arr[i]);
		if (ret) {
			hwlog_err("%s set fod register[%d] failed\n", __func__, i);
			return -1;
		}
	}

	return 0;
}

static int idtp9221_rx_fod_coef_5v(void)
{
	int i;
	int ret;
	int fod_val;
	u16 chip_id = 0;
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s: para is null\n", __func__);
		return -1;
	}

	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return ret;

	if (chip_id == ERR_CHIP_ID) {
		hwlog_err("%s: get chip id fail\n", __func__);
		return -EINVAL;
	}
	g_chip_id = chip_id;
	/* set rx 5v fod coef */
	for (i = 0; i < IDT9221_RX_FOD_COEF_LEN; i++) {
		if (chip_id == STWLC33_CHIP_ID)
			fod_val = di->st_rx_fod_5v[i];
		else
			fod_val = di->rx_fod_5v[i];

		ret = idtp9221_write_word(IDT9221_RX_FOD_COEF_STSRT_ADDR +
			i * WORD_LEN, fod_val);
		if (ret) {
			hwlog_err("%s: set rx_fod_5v reg[%d] failed\n", __func__, i);
			return -1;
		}
	}
	hwlog_info("[%s] update ok", __func__);
	return 0;
}

static int idtp9221_rx_fod_coef_9v(void)
{
	int i;
	int ret;
	int fod_val;
	u16 chip_id = 0;
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s: para is null\n", __func__);
		return -1;
	}
	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return ret;

	if (chip_id == ERR_CHIP_ID) {
		hwlog_err("%s: get chip id fail\n", __func__);
		return -EINVAL;
	}
	/* set rx 9v fod coef */
	for (i = 0; i < IDT9221_RX_FOD_COEF_LEN; i++) {
		if (chip_id == STWLC33_CHIP_ID)
			fod_val = di->st_rx_fod_9v[i];
		else
			fod_val = di->rx_fod_9v[i];
		ret = idtp9221_write_word(IDT9221_RX_FOD_COEF_STSRT_ADDR +
			i * WORD_LEN, fod_val);
		if (ret) {
			hwlog_err("%s: set rx_fod_9v reg[%d] failed\n", __func__, i);
			return -1;
		}
	}
	hwlog_info("[%s] update ok", __func__);
	return 0;
}

static int idtp9221_rx_fod_coef_12v(void)
{
	int i;
	int ret;
	int fod_val;
	u16 chip_id = 0;
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s: para is null\n", __func__);
		return -1;
	}
	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return ret;
	if (chip_id == ERR_CHIP_ID) {
		hwlog_err("%s: get chip id fail\n", __func__);
		return -EINVAL;
	}
	/* set rx 12v fod coef */
	for (i = 0; i < IDT9221_RX_FOD_COEF_LEN; i++) {
		if (chip_id == STWLC33_CHIP_ID)
			fod_val = di->st_rx_fod_12v[i];
		else
			fod_val = di->rx_fod_12v[i];
		ret = idtp9221_write_word(IDT9221_RX_FOD_COEF_STSRT_ADDR +
			i * WORD_LEN, fod_val);
		if (ret) {
			hwlog_err("%s: set rx_fod_12v reg[%d] failed\n", __func__, i);
			return -1;
		}
	}
	hwlog_info("[%s] update ok", __func__);
	return 0;
}

static int idtp9221_chip_init(unsigned int tx_vset)
{
	int ret = 0;
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s: para is null\n", __func__);
		return -1;
	}
	switch (tx_vset) {
	case WIRELESS_CHIP_INIT:
		hwlog_info("[%s] DEFAULT_CHIP_INIT\n", __func__);
		/* set wdt */
		ret |= idtp9221_write_word(IDT9221_RX_FC_TIMEOUT_ADDR, IDT9221_RX_FC_TIMEOUT);
		/* set vrect coef */
		ret |= idtp9221_write_byte(IDT9221_RX_PWR_KNEE_ADDR, 12);
		ret |= idtp9221_write_byte(IDT9221_RX_VRCORR_FACTOR_ADDR, 16);
		ret |= idtp9221_write_word(IDT9221_RX_VRMAX_COR_ADDR, 58);
		ret |= idtp9221_write_word(IDT9221_RX_VRMIN_COR_ADDR, 10);
		/* CAP Enable */
		ret |= idtp9221_write_byte(IDT9221_RX_USER_FLAGS_ADDR, IDT9221_RX_USER_FLAGS);
	case ADAPTER_5V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 5V_CHIP_INIT\n", __func__);
		ret |= idtp9221_rx_fod_coef_5v();
		break;
	case ADAPTER_9V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 9V_CHIP_INIT\n", __func__);
		ret |= idtp9221_rx_fod_coef_9v();
		break;
	case WILREESS_SC_CHIP_INIT:
		hwlog_info("[%s] SC_CHIP_INIT\n", __func__);
		/* set vrect coef */
		ret |= idtp9221_write_byte(IDT9221_RX_PWR_KNEE_ADDR, 18);
		ret |= idtp9221_write_byte(IDT9221_RX_VRCORR_FACTOR_ADDR, 50);
		ret |= idtp9221_write_word(IDT9221_RX_VRMAX_COR_ADDR,
			di->sc_rx_vrmax_gap);
		ret |= idtp9221_write_word(IDT9221_RX_VRMIN_COR_ADDR, 10);
		if (!ret)
			ret |= idtp9221_rx_fod_coef_9v();
		break;
	case ADAPTER_12V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 12V_CHIP_INIT\n", __func__);
		ret |= idtp9221_rx_fod_coef_12v();
		break;
	default:
		hwlog_info("%s: input para is invalid\n", __func__);
		break;
	}
	return ret;
}

static int stwlc33_chip_init(unsigned int tx_vset)
{
	int ret = 0;

	switch (tx_vset) {
	case WIRELESS_CHIP_INIT:
		hwlog_info("[%s] DEFAULT_CHIP_INIT\n", __func__);
		/* ldo drop */
		ret |= idtp9221_write_byte(STWLC33_LDO_DROP0_ADDR,
			STWLC33_LDO_DROP0_VAL);
		ret |= idtp9221_write_byte(STWLC33_LDO_DROP1_ADDR,
			STWLC33_LDO_DROP1_VAL);
		ret |= idtp9221_write_byte(STWLC33_LDO_DROP2_ADDR,
			STWLC33_LDO_DROP2_VAL);
		ret |= idtp9221_write_byte(STWLC33_LDO_DROP3_ADDR,
			STWLC33_LDO_DROP3_VAL);
		/* ldo cur thres */
		ret |= idtp9221_write_byte(STWLC33_LDO_CUR_TH1_ADDR,
			STWLC33_LDO_CUR_TH1_VAL);
		ret |= idtp9221_write_byte(STWLC33_LDO_CUR_TH2_ADDR,
			STWLC33_LDO_CUR_TH2_VAL);
		ret |= idtp9221_write_byte(STWLC33_LDO_CUR_TH3_ADDR,
			STWLC33_LDO_CUR_TH3_VAL);
		/* fall through */
	case ADAPTER_5V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 5V_CHIP_INIT\n", __func__);
		ret |= idtp9221_rx_fod_coef_5v();
		break;
	case ADAPTER_9V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 9V_CHIP_INIT\n", __func__);
		ret |= idtp9221_rx_fod_coef_9v();
		break;
	case WILREESS_SC_CHIP_INIT:
		hwlog_info("[%s] SC_CHIP_INIT\n", __func__);
		/* ldo drop */
		ret |= idtp9221_write_byte(STWLC33_LDO_DROP0_ADDR,
			STWLC33_LDO_DROP0_SC_VAL);
		ret |= idtp9221_write_byte(STWLC33_LDO_DROP1_ADDR,
			STWLC33_LDO_DROP1_SC_VAL);
		ret |= idtp9221_write_byte(STWLC33_LDO_DROP2_ADDR,
			STWLC33_LDO_DROP2_SC_VAL);
		ret |= idtp9221_write_byte(STWLC33_LDO_DROP3_ADDR,
			STWLC33_LDO_DROP3_SC_VAL);
		/* ldo cur thres */
		ret |= idtp9221_write_byte(STWLC33_LDO_CUR_TH1_ADDR,
			STWLC33_LDO_CUR_TH1_SC_VAL);
		ret |= idtp9221_write_byte(STWLC33_LDO_CUR_TH2_ADDR,
			STWLC33_LDO_CUR_TH2_SC_VAL);
		ret |= idtp9221_write_byte(STWLC33_LDO_CUR_TH3_ADDR,
			STWLC33_LDO_CUR_TH3_SC_VAL);
		if (!ret)
			ret |= idtp9221_rx_fod_coef_9v();
		break;
	case ADAPTER_12V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 12V_CHIP_INIT\n", __func__);
		ret |= idtp9221_rx_fod_coef_12v();
		break;
	default:
		hwlog_info("%s: input para is invalid\n", __func__);
		break;
	}
	return ret;
}

static int idt_st_chip_init(unsigned int tx_vset, unsigned int tx_type, void *dev_data)
{
	int ret;
	u16 chip_id = 0;

	hwlog_info("tx_type=%d\n", tx_type);

	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return ret;

	if (chip_id == STWLC33_CHIP_ID)
		return stwlc33_chip_init(tx_vset);

	return idtp9221_chip_init(tx_vset);
}

static void idtp9221_stop_charging(void *dev_data)
{
	struct idtp9221_device_info *di = dev_data;

	if (!di) {
		hwlog_err("%s: para is null\n", __func__);
		return;
	}

	stop_charging_flag = 1;

	if (!irq_abnormal_flag)
		return;

	if (wlrx_get_wired_channel_state() != WIRED_CHANNEL_ON) {
		hwlog_info("[stop_charging] irq_abnormal,keep rx_sw on\n");
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, true);
	} else {
		di->irq_cnt = 0;
		irq_abnormal_flag = false;
		idtp9221_enable_irq(di);
		hwlog_info("[%s] wired channel on, enable irq_int\n", __func__);
	}
}

static u8 *idtp9221_get_otp_fw_version(void)
{
	int ret, i;
	static u8 fw_version[IDT9221_RX_OTP_FW_VERSION_STRING_LEN] = {0};
	u8 data[IDT9221_RX_OTP_FW_VERSION_LEN] = {0};
	u8 buff[IDT9221_RX_TMP_BUFF_LEN] = {0};
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return "error";
	}
	memset(fw_version, 0, IDT9221_RX_OTP_FW_VERSION_STRING_LEN);
	ret = idtp9221_read_block(di, IDT9221_RX_OTP_FW_VERSION_ADDR,
		data, IDT9221_RX_OTP_FW_VERSION_LEN);
	if (!ret) {
		for (i = IDT9221_RX_OTP_FW_VERSION_LEN - 1; i >= 0; i--) {
			if (i != 0)
				snprintf(buff, IDT9221_RX_TMP_BUFF_LEN, "0x%02x ", data[i]);
			else
				snprintf(buff, IDT9221_RX_TMP_BUFF_LEN, "0x%02x", data[i]);
			strncat(fw_version, buff, strlen(buff));
		}
		return fw_version;
	} else {
		return "error";
	}
}

static void stwlc33_get_rx_chip_temp(void)
{
	int ret;
	int count = 0;
	u16 chip_temp = 0;
	u16 chip_id = 0;

	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return;

	if (chip_id != STWLC33_CHIP_ID)
		return;
	do {
		ret = idtp9221_read_word(STWLC33_RX_CHIP_TEMP_ADDR, &chip_temp);
		if (ret) {
			hwlog_err("%s: read fail\n", __func__);
			break;
		}
		count++;
		/* (ADC code-1350)*(83/444)-273 */
		chip_temp = (chip_temp - 1350) * 83 / 444 - 273;
		hwlog_info("[%s] chip temp:%d", __func__, chip_temp);
	} while (count < STWLC33_GET_RX_CHIP_TEMP_RETRY_CNT);
}

static u8 *stwlc33_get_otp_fw_version(void)
{
	int i;
	int ret;
	static u8 fw_version[IDT9221_RX_OTP_FW_VERSION_STRING_LEN];
	u8 buff[IDT9221_RX_TMP_BUFF_LEN] = {0};
	u8 data[STWLC33_RX_OTP_FW_VERSION_LEN] = { 0 };
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return "error";
	}
	memset(fw_version, 0, IDT9221_RX_OTP_FW_VERSION_STRING_LEN);
	ret = idtp9221_read_block(di, STWLC33_RX_OTP_FW_VERSION_ADDR,
		data, STWLC33_RX_OTP_FW_VERSION_LEN);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return "error";
	}
	strncat(fw_version, IDT9221_OTP_FW_HEAD, strlen(IDT9221_OTP_FW_HEAD));
	for (i = STWLC33_RX_OTP_FW_VERSION_LEN - 1; i >= 0; i--) {
		snprintf(buff, IDT9221_RX_TMP_BUFF_LEN, " 0x%02x", data[i]);
		strncat(fw_version, buff, strlen(buff));
	}

	hwlog_info("[%s] otp version:%s\n", __func__, fw_version);
	return fw_version;
}

static u8 *idt_st_get_otp_fw_version(void)
{
	int ret;
	u16 chip_id = 0;

	g_i2c_fail_cnt = 0;
	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return "error i2c";

	if (chip_id == STWLC33_CHIP_ID)
		return stwlc33_get_otp_fw_version();
	else if (chip_id == IDTP9221_CHIP_ID)
		return idtp9221_get_otp_fw_version();
	else
		return "error";
}

static int idt_st_get_chip_info_str(char *info_str, int len, void *dev_data)
{
	int ret;
	u16 chip_id = 0;

	if (!info_str || (len < WLTRX_IC_CHIP_INFO_LEN))
		return -EINVAL;

	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return -EIO;

	if (chip_id == IDTP9221_CHIP_ID)
		return snprintf(info_str, WLTRX_IC_CHIP_INFO_LEN,
			"chip_id:idtp9221");

	return snprintf(info_str, WLTRX_IC_CHIP_INFO_LEN, "chip_id:stwlc33");
}

static int idtp9221_check_fwupdate(unsigned int fw_sram_mode,
	struct idtp9221_device_info *di)
{
	int ret;
	int i;
	u8 *otp_fw_version;
	unsigned int fw_update_size;

	fw_update_size = ARRAY_SIZE(fw_update);
	otp_fw_version = idt_st_get_otp_fw_version();
	if (strcmp(otp_fw_version, "error") == 0) {
		hwlog_err("%s:get firmware version fail\n", __func__);
		return -1;
	}

	for (i = 0; i < fw_update_size; i++) {
		if ((fw_sram_mode == fw_update[i].fw_sram_mode) &&
			(strcmp(otp_fw_version, fw_update[i].name_fw_update_from) >= 0) &&
			(strcmp(otp_fw_version, fw_update[i].name_fw_update_to) <= 0)) {
			hwlog_info("[%s] SRAM update start! otp_fw_version = %s\n", __func__, otp_fw_version);
			ret = idt_st_program_sramupdate(&fw_update[i]);
			if (ret) {
				hwlog_err("%s: SRAM update fail\n", __func__);
				return -1;
			}
			otp_fw_version = idt_st_get_otp_fw_version();
			hwlog_info("[%s] SRAM update succ! otp_fw_version = %s\n", __func__, otp_fw_version);
			if (g_st33_need_rst_flag) {
				g_st33_need_rst_flag = false;
				idtp9221_chip_reset(di);
			}
			return 0;
		}
	}
	hwlog_info("[%s] SRAM no need update, otp_fw_version = %s\n", __func__, otp_fw_version);
	return -1;
}

static int idtp9221_rx_fwupdate(void *dev_data)
{
	struct idtp9221_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return idtp9221_check_fwupdate(WIRELESS_RX, di);
}

static int idtp9221_tx_fwupdate(void *dev_data)
{
	struct idtp9221_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return idtp9221_check_fwupdate(WIRELESS_TX, di);
}

static int idtp9221_data_received_handler(struct idtp9221_device_info *di)
{
	int ret;
	int i;
	u8 cmd;
	u8 buff[QI_PKT_LEN] = { 0 };

	ret = idtp9221_read_block(di, IDT9221_TX_TO_RX_HEADER_ADDR,
		buff, QI_PKT_LEN);
	if (ret) {
		hwlog_err("data_received_handler: read received data failed\n");
		return -WLC_ERR_I2C_R;
	}

	cmd = buff[QI_PKT_CMD];
	hwlog_info("[data_received_handler] cmd: 0x%x\n", cmd);
	for (i = QI_PKT_DATA; i < QI_PKT_LEN; i++)
		hwlog_info("[data_received_handler] data: 0x%x\n", buff[i]);

	switch (cmd) {
	case QI_CMD_TX_ALARM:
	case QI_CMD_ACK_BST_ERR:
		di->irq_val &= ~IDT9221_RX_STATUS_TXDATA_RECEIVED;
		if (g_idtp9221_handle &&
			g_idtp9221_handle->hdl_non_qi_fsk_pkt)
			g_idtp9221_handle->hdl_non_qi_fsk_pkt(buff, QI_PKT_LEN, di);
		break;
	default:
		break;
	}

	return 0;
}

static void stwlc33_iout_calibration(void)
{
	int ret;
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
	usleep_range(4500, 5000); /* 5ms */
	gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	ret = idtp9221_write_byte(STWLC33_IOUT_CALI_ADDR,
		STWLC33_IOUT_CALI_VAL);
	if (ret)
		hwlog_err("%s: write reg failed\n", __func__);
}

static void stwlc33_need_rst_check(void)
{
	int ret;
	u16 chip_id = 0;

	ret = idtp9221_get_chip_id(&chip_id);
	if (ret)
		return;

	if (chip_id == STWLC33_CHIP_ID)
		g_st33_need_rst_flag = true;
}

static void idtp9221_rx_ready_handler(struct idtp9221_device_info *di)
{
	int ret;
	u16 chip_id = 0;

	ret = idtp9221_get_chip_id(&chip_id);
	if (ret < 0) {
		hwlog_err("%s: get chip_id failed\n", __func__);
		return;
	}
	if (wlrx_get_wired_channel_state() != WIRED_CHANNEL_ON) {
		hwlog_info("%s rx ready, goto wireless charging\n", __func__);
		stop_charging_flag = 0;
		di->irq_cnt = 0;
		wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);
		if (chip_id == STWLC33_CHIP_ID)
			stwlc33_iout_calibration();

		msleep(CHANNEL_SW_TIME);
		if (chip_id == STWLC33_CHIP_ID) {
			ret = idtp9221_read_word(STWLC33_IOUT_ADC_VAL_ADDR,
				&stwlc33_iout_adc_val);
			if (ret) {
				stwlc33_iout_adc_val = 0;
				hwlog_err("%s: get cl val failed\n", __func__);
			}
		}
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_READY, NULL);
	}
}

static void idtp9221_handle_abnormal_irq(struct idtp9221_device_info *di)
{
	static struct timespec64 ts64_timeout;
	struct timespec64 ts64_interval;
	struct timespec64 ts64_now;

	ts64_now = current_kernel_time64();
	ts64_interval.tv_sec = 0;
	ts64_interval.tv_nsec = WIRELESS_INT_TIMEOUT_TH * NSEC_PER_MSEC;

	hwlog_info("[%s] irq_cnt = %d\n", __func__, ++di->irq_cnt);
	/* power on interrupt happen first time, so start monitor it! */
	if (di->irq_cnt == 1) {
		ts64_timeout = timespec64_add_safe(ts64_now, ts64_interval);
		if (ts64_timeout.tv_sec == TIME_T_MAX) {
			di->irq_cnt = 0;
			hwlog_err("%s: time overflow happend\n", __func__);
			return;
		}
	}

	if (timespec64_compare(&ts64_now, &ts64_timeout) >= 0) {
		if (di->irq_cnt >= WIRELESS_INT_CNT_TH) {
			irq_abnormal_flag = true;
			wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, true);
			idtp9221_disable_irq_nosync(di);
			gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
			hwlog_err("%s: more than %d interrupts happened in %ds, disable irq\n",
				__func__, WIRELESS_INT_CNT_TH,
				WIRELESS_INT_TIMEOUT_TH / WL_MSEC_PER_SEC);
		} else {
			di->irq_cnt = 0;
			hwlog_info("%s: less than %d interrupts happened in %ds, clear irq count\n",
				__func__, WIRELESS_INT_CNT_TH,
				WIRELESS_INT_TIMEOUT_TH / WL_MSEC_PER_SEC);
		}
	}
}

static void idtp9221_rx_power_on_handler(struct idtp9221_device_info *di)
{
	u8 rx_ss = 0; /* ss: Signal Strength */
	int pwr_flag = RX_PWR_ON_NOT_GOOD;

	idtp9221_handle_abnormal_irq(di);
	idtp9221_read_byte(IDT9221_RX_SS_ADDR, &rx_ss);
	hwlog_info("[rx_power_on_handler] SS = %u\n", rx_ss);
	if ((rx_ss > di->rx_ss_good_lth) && (rx_ss != IDT9221_RX_SS_MAX))
		pwr_flag = RX_PWR_ON_GOOD;
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_PWR_ON, &pwr_flag);
}

static int idtp9221_tx_stop_config(void *dev_data)
{
	return 0;
}

static int idtp9221_enable_tx_mode(bool enable, void *dev_data)
{
	int ret;

	if (enable)
		ret = idtp9221_write_word_mask(IDT9221_CMD3_ADDR,
			IDT9221_CMD3_TX_EN_MASK, IDT9221_CMD3_TX_EN_SHIFT, 1);
	else
		ret = idtp9221_write_word_mask(IDT9221_CMD3_ADDR,
			IDT9221_CMD3_TX_DIS_MASK, IDT9221_CMD3_TX_DIS_SHIFT, 1);

	if (ret) {
		hwlog_err("%s: %s tx mode fail\n", __func__, enable ? "enable" : "disable");
		return -1;
	}
	return 0;
}

static int idtp9221_get_tx_iin(u16 *tx_iin, void *dev_data)
{
	int ret;

	ret = idtp9221_read_word(IDT9221_TX_IIN_ADDR, tx_iin);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	return 0;
}

static int idtp9221_get_tx_vrect(u16 *tx_vrect, void *dev_data)
{
	int ret;

	ret = idtp9221_read_word(IDT9221_TX_VRECT_ADDR, tx_vrect);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	return 0;
}

static int idtp9221_get_tx_vin(u16 *tx_vin, void *dev_data)
{
	int ret;
	u16 chip_id = 0;

	ret = idtp9221_read_word(IDT9221_TX_VIN_ADDR, tx_vin);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	ret = idtp9221_get_chip_id(&chip_id);
	if (!ret && (chip_id == STWLC33_CHIP_ID) && !(*tx_vin))
		*tx_vin = TX_DEFAULT_VOUT;

	return 0;
}

static int idtp9221_get_tx_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	ret = idtp9221_read_word(IDT9221_TX_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	if (data)
		*fop = 60000 / data; /* Ping freq(khz) = 60000/PingPeriod */
	return 0;
}

static int idtp9221_set_tx_max_fop(u16 fop, void *dev_data)
{
	int ret;

	if (!fop)
		return -1;
	/* Ping freq(khz) = 60000 / PingPeriod */
	ret = idtp9221_write_word(IDT9221_TX_MAX_FOP_ADDR, 60000 / fop);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return 0;
}

static int idtp9221_get_tx_max_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	ret = idtp9221_read_word(IDT9221_TX_MAX_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	if (data)
		*fop = 60000 / data; /* Ping freq(khz) = 60000/PingPeriod */
	return 0;
}

static int idtp9221_set_tx_min_fop(u16 fop, void *dev_data)
{
	int ret;

	if (!fop)
		return -1;
	/* Ping freq(khz) = 60000 / PingPeriod */
	ret = idtp9221_write_word(IDT9221_TX_MIN_FOP_ADDR, 60000 / fop);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return 0;
}

static int idtp9221_get_tx_min_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	ret = idtp9221_read_word(IDT9221_TX_MIN_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	if (data)
		*fop = 60000 / data; /* Ping freq(khz) = 60000/PingPeriod */
	return 0;
}

static int idtp9221_set_tx_ping_freq(u16 ping_freq, void *dev_data)
{
	int ret;

	if ((ping_freq < IDT9221_TX_PING_FREQUENCY_MIN) ||
		(ping_freq > IDT9221_TX_PING_FREQUENCY_MAX)) {
		hwlog_err("%s: ping frequency is out of range\n", __func__);
		return -1;
	}
	/* Ping freq(khz) = 60000 / PingPeriod */
	ret = idtp9221_write_word(IDT9221_TX_PING_FREQUENCY_ADDR, 60000 / ping_freq);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return ret;
}

static int idtp9221_get_tx_ping_freq(u16 *ping_freq, void *dev_data)
{
	int ret;
	u16 data = 0;

	ret = idtp9221_read_word(IDT9221_TX_PING_FREQUENCY_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	if (data)
		*ping_freq = 60000 / data; /* Ping freq(khz) = 60000/PingPeriod */
	return 0;
}

static int idtp9221_set_tx_ping_interval(u16 ping_interval, void *dev_data)
{
	int ret;

	if ((ping_interval < IDT9221_TX_PING_INTERVAL_MIN) ||
		(ping_interval > IDT9221_TX_PING_INTERVAL_MAX)) {
		hwlog_err("%s: ping interval is out of range\n", __func__);
		return -1;
	}
	ret = idtp9221_write_byte(IDT9221_TX_PING_INTERVAL_ADDR,
		(u8)(ping_interval / IDT9221_TX_PING_INTERVAL_STEP));
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return 0;
}

static int idtp9221_get_tx_ping_interval(u16 *ping_interval, void *dev_data)
{
	int ret;
	u8 data = 0;

	ret = idtp9221_read_byte(IDT9221_TX_PING_INTERVAL_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	*ping_interval = data * IDT9221_TX_PING_INTERVAL_STEP;
	return 0;
}

static int idtp9221_get_chip_temp(s16 *chip_temp, void *dev_data)
{
	int ret;

	ret = idtp9221_read_byte(IDT9221_CHIP_TEMP_ADDR, (u8 *)chip_temp);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	return 0;
}

static int idtp9221_tx_chip_init(unsigned int client, void *dev_data)
{
	int ret;
	struct idtp9221_device_info *di = dev_data;

	if (!di)
		return -1;

	di->irq_cnt = 0;
	irq_abnormal_flag = false;
	idtp9221_enable_irq(di);
	ret = idtp9221_write_word(IDT9221_TX_OCP_ADDR, IDT9221_TX_OCP_VAL);
	ret += idtp9221_write_word(IDT9221_TX_OVP_ADDR, IDT9221_TX_OVP_VAL);
	ret += idtp9221_write_word_mask(IDT9221_CMD3_ADDR,
		IDT9221_CMD3_TX_FOD_EN_MASK, IDT9221_CMD3_TX_FOD_EN_SHIFT, 1);
	ret += idtp9221_write_word(IDT9221_TX_FOD_THD0_ADDR, di->tx_fod_th_5v);
	ret += idtp9221_set_tx_min_fop(IDT9221_TX_MIN_FOP_VAL, di);
	ret += idtp9221_set_tx_max_fop(IDT9221_TX_MAX_FOP_VAL, di);
	ret += idtp9221_set_tx_ping_freq(IDT9221_TX_PING_FREQUENCY_INIT, di);
	ret += idtp9221_set_tx_ping_interval(IDT9221_TX_PING_INTERVAL_INIT, di);

	return ret;
}

static int idtp9221_send_fsk_msg(u8 cmd, u8 *data, int data_len,
	void *dev_data)
{
	int ret;
	u8 msg_byte_num;
	u8 write_data[IDT9221_TX_TO_RX_DATA_LEN + IDT9221_ADDR_LEN] = { 0 };
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!di) {
		hwlog_err("%s: para is null\n", __func__);
		return -1;
	}

	if ((data_len > IDT9221_TX_TO_RX_DATA_LEN) || (data_len < 0)) {
		hwlog_err("%s: send data out of range\n", __func__);
		return -1;
	}

	if (cmd == IDT9221_CMD_ACK)
		msg_byte_num = IDT9221_CMD_ACK_HEAD;
	else
		msg_byte_num = idtp9221_send_fsk_msg_len[data_len + 1];

	ret = idtp9221_write_byte(IDT9221_TX_TO_RX_HEADER_ADDR, msg_byte_num);
	if (ret) {
		hwlog_err("%s: write header failed\n", __func__);
		return -1;
	}
	ret = idtp9221_write_byte(IDT9221_TX_TO_RX_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s: write cmd failed\n", __func__);
		return -1;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data + IDT9221_ADDR_LEN, data, data_len);
		ret = idtp9221_write_block(di, IDT9221_TX_TO_RX_DATA_ADDR, write_data, data_len);
		if (ret) {
			hwlog_err("%s: write data into fsk register failed\n", __func__);
			return -1;
		}
	}
	ret = idtp9221_write_word_mask(IDT9221_CMD3_ADDR,
		IDT9221_CMD3_TX_SEND_FSK_MASK, IDT9221_CMD3_TX_SEND_FSK_SHIFT, 1);
	if (ret) {
		hwlog_err("%s: send fsk failed\n", __func__);
		return -1;
	}

	hwlog_info("[%s] success\n", __func__);
	return 0;
}

static bool idtp9221_check_rx_disconnect(void *dev_data)
{
	struct idtp9221_device_info *di = dev_data;

	if (!di)
		return true;

	if (di->ept_type & IDT9221_TX_EPT_CEP_TIMEOUT) {
		di->ept_type &= ~IDT9221_TX_EPT_CEP_TIMEOUT;
		hwlog_info("[%s] RX disconnect\n", __func__);
		return true;
	}
	return false;
}

static bool idtp9221_in_tx_mode(void *dev_data)
{
	u8 mode;
	int ret = idtp9221_get_mode(&mode);

	if (ret) {
		hwlog_err("%s: get mode failed\n", __func__);
		return false;
	}
	if (mode & IDT9221_TX_WPCMODE)
		return true;
	return false;
}

static bool idtp9221_in_rx_mode(void *dev_data)
{
	return idtp9221_is_tx_exist(dev_data);
}

static int idtp9221_set_tx_open_flag(bool enable, void *dev_data)
{
	if (enable)
		g_i2c_fail_cnt = 0;

	return 0;
}

static int idtp9221_get_tx_ept_type(u16 *ept_type)
{
	int ret;
	u16 data = 0;

	ret = idtp9221_read_word(IDT9221_TX_EPT_TYPE_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	*ept_type = data;
	hwlog_info("[%s] EPT type = 0x%04x", __func__, *ept_type);
	ret = idtp9221_write_word(IDT9221_TX_EPT_TYPE_ADDR, 0);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return 0;
}

static int idtp9221_get_ask_packet(u8 *packet_data, int packet_data_len,
	void *dev_data)
{
	int ret;
	int i;
	char buff[IDT9221_RX_TO_TX_PACKET_BUFF_LEN] = { 0 };
	char packet_str[IDT9221_RX_TO_TX_PACKET_STR_LEN] = { 0 };
	struct idtp9221_device_info *di = g_idtp9221_di;

	if (!packet_data || (packet_data_len < IDT9221_RX_TO_TX_PACKET_LEN)) {
		hwlog_err("%s: NULL pointer", __func__);
		return -1;
	}
	ret = idtp9221_read_block(di, IDT9221_RX_TO_TX_HEADER_ADDR,
		packet_data, IDT9221_RX_TO_TX_PACKET_LEN);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	for (i = 0; i < IDT9221_RX_TO_TX_PACKET_LEN; i++) {
		snprintf(buff, IDT9221_RX_TO_TX_PACKET_BUFF_LEN, "0x%02x ", packet_data[i]);
		strncat(packet_str, buff, strlen(buff));
	}
	hwlog_info("[%s] RX back packet: %s\n", __func__, packet_str);
	return 0;
}

static void idtp9221_handle_tx_ept(struct idtp9221_device_info *di)
{
	int ret;

	ret = idtp9221_get_tx_ept_type(&di->ept_type);
	if (ret) {
		hwlog_err("%s: get tx ept type failed\n", __func__);
		return;
	}
	switch (di->ept_type) {
	case IDT9221_TX_EPT_CMD:
		di->ept_type &= ~IDT9221_TX_EPT_CMD;
		hwlog_info("[%s] ept command\n", __func__);
		break;
	case IDT9221_TX_EPT_CEP_TIMEOUT:
		di->ept_type &= ~IDT9221_TX_EPT_CEP_TIMEOUT;
		hwlog_info("[%s] RX disconnect\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX, POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	case IDT9221_TX_EPT_FOD:
		di->ept_type &= ~IDT9221_TX_EPT_FOD;
		hwlog_info("[%s] fod happened\n", __func__);
		break;
	case IDT9221_TX_EPT_OVP:
		di->ept_type &= ~IDT9221_TX_EPT_OVP;
		hwlog_info("[%s] ovp happened\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX, POWER_NE_WLTX_OVP, NULL);
		break;
	case IDT9221_TX_EPT_OCP:
		di->ept_type &= ~IDT9221_TX_EPT_OCP;
		hwlog_info("[%s] ocp happened\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX, POWER_NE_WLTX_OCP, NULL);
		break;
	case IDT9221_TX_EPT_TIMEOUT:
		di->ept_type &= ~IDT9221_TX_EPT_TIMEOUT;
		hwlog_info("[%s] chip reset happened\n", __func__);
		break;
	default:
		break;
	}
}

static void stwlc33_rx_ready_check_work(struct work_struct *work)
{
	int i;
	int vrect = 0;
	int sum = 0;
	int avg_val;
	int sample_cnt = 0;
	struct idtp9221_device_info *di = NULL;

	di = g_idtp9221_di;
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		goto ready_check_end;
	}
	for (i = 0; i < STWLC33_RX_VRECT_READ_CNT; i++) {
		if (stop_charging_flag) {
			hwlog_err("%s: already stop charging\n", __func__);
			goto ready_check_end;
		}
		msleep(STWLC33_VRECT_SLEEP_TIME);
		if (i < STWLC33_VRECT_IGNORE_CNT) /* ignore first 1.5s */
			continue;
		(void)idtp9221_get_rx_vrect(&vrect, di);
		if (vrect <= 0) {
			hwlog_err("%s: get rx vrect err\n", __func__);
			goto ready_check_end;
		}
		sum += vrect;
		sample_cnt++;
	}
	if (!sample_cnt)
		avg_val = STWLC33_VRECT_INVALID_VAL;
	else
		avg_val = sum / sample_cnt;
	hwlog_info("[%s] avg vrect is %dmV\n", __func__, avg_val);
	if (avg_val >= STWLC33_VRECT_THRES_VAL) {
		idtp9221_rx_ready_handler(di);
	} else {
		idtp9221_disable_irq_nosync(di);
		hwlog_err("%s: ldo off\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_LDO_OFF, NULL);
		irq_abnormal_flag = true;
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, true);
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	}
	stwlc33_rx_ready_check_flag = 0;
	return;

ready_check_end:
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_LDO_OFF, NULL);
	stwlc33_rx_ready_check_flag = 0;
}

static int stwlc33_rx_ready_check_handler(struct idtp9221_device_info *di)
{
	if ((di->irq_val & IDT9221_RX_STATUS_READY) ||
		(di->irq_val & IDT9221_RX_STATUS_POWER_ON)) {
		di->irq_val &= ~IDT9221_RX_STATUS_READY;
		di->irq_val &= ~IDT9221_RX_STATUS_POWER_ON;
		if (stwlc33_rx_ready_check_flag) {
			hwlog_err("%s: rx_vrect_work running\n",
				__func__);
			return -1;
		}
		stwlc33_rx_ready_check_flag = 1;
		idtp9221_sleep_enable(true, di);
		idtp9221_rx_power_on_handler(di);
		stop_charging_flag = 0;
		schedule_work(&di->rx_ready_check_work);
	}

	return 0;
}

static int idtp9221_clear_tx_interrupt(u16 itr)
{
	int ret;

	ret = idtp9221_write_word(IDT9221_RX_INT_CLEAR_ADDR, itr);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}

	ret = idtp9221_write_word_mask(IDT9221_CMD3_ADDR,
		IDT9221_CMD3_TX_CLRINT_MASK, IDT9221_CMD3_TX_CLRINT_SHIFT, 1);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return 0;
}

static void idtp9221_tx_mode_irq_handler(struct idtp9221_device_info *di)
{
	int ret;
	u16 irq_value = 0;

	ret = idtp9221_read_word(IDT9221_RX_INT_STATUS_ADDR, &di->irq_val);
	if (ret) {
		hwlog_err("%s: read interrupt fail, clear interrupt\n", __func__);
		idtp9221_clear_tx_interrupt(WORD_MASK);
		goto FuncEnd;
	}

	hwlog_info("[%s] interrupt 0x%04x\n", __func__, di->irq_val);
	idtp9221_clear_tx_interrupt(di->irq_val);

	/* receice message from RX, please handler it! */
	if (di->irq_val & IDT9221_TX_STATUS_START_DPING) {
		hwlog_info("%s: tx PING interrupt\n", __func__);
		di->irq_val &= ~IDT9221_TX_STATUS_START_DPING;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_PING_RX, NULL);
	}

	if (di->irq_val & IDT9221_TX_STATUS_GET_SS) {
		hwlog_info("%s: Signal Strength packet interrupt\n", __func__);
		di->irq_val &= ~IDT9221_TX_STATUS_GET_SS;
		if (g_idtp9221_handle && g_idtp9221_handle->hdl_qi_ask_pkt)
			g_idtp9221_handle->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & IDT9221_TX_STATUS_GET_ID) {
		hwlog_info("%s: ID packet interrupt\n", __func__);
		di->irq_val &= ~IDT9221_TX_STATUS_GET_ID;
		if (g_idtp9221_handle && g_idtp9221_handle->hdl_qi_ask_pkt)
			g_idtp9221_handle->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & IDT9221_TX_STATUS_GET_CFG) {
		hwlog_info("%s: Config packet interrupt\n", __func__);
		di->irq_val &= ~IDT9221_TX_STATUS_GET_CFG;
		if (g_idtp9221_handle && g_idtp9221_handle->hdl_qi_ask_pkt)
			g_idtp9221_handle->hdl_qi_ask_pkt(di);
		power_event_bnc_notify(POWER_BNT_WLTX, POWER_NE_WLTX_GET_CFG, NULL);
	}
	if (di->irq_val & IDT9221_TX_STATUS_EPT_TYPE) {
		di->irq_val &= ~IDT9221_TX_STATUS_EPT_TYPE;
		idtp9221_handle_tx_ept(di);
	}

	if (di->irq_val & IDT9221_TX_STATUS_GET_PPP) {
		hwlog_info("[%s] TX receive personal property ASK packet\n", __func__);
		di->irq_val &= ~IDT9221_TX_STATUS_GET_PPP;
		if (g_idtp9221_handle && g_idtp9221_handle->hdl_non_qi_ask_pkt)
			g_idtp9221_handle->hdl_non_qi_ask_pkt(di);
	}
FuncEnd:
	/* clear interrupt again */
	if (!gpio_get_value(di->gpio_int)) {
		idtp9221_read_word(IDT9221_RX_INT_STATUS_ADDR, &irq_value);
		hwlog_info("[%s] gpio_int is low, clear interrupt again! irq_value = 0x%x\n", __func__, irq_value);
		idtp9221_clear_tx_interrupt(WORD_MASK);
	}
}

static void idtp9221_rx_mode_irq_handler(struct idtp9221_device_info *di)
{
	int ret;
	u16 chip_id = 0;
	u16 irq_value = 0;

	ret = idtp9221_read_word(IDT9221_RX_INT_STATUS_ADDR, &di->irq_val);
	if (ret) {
		hwlog_err("%s read interrupt fail, clear interrupt\n", __func__);
		idtp9221_clear_interrupt(WORD_MASK);
		idtp9221_handle_abnormal_irq(di);
		goto FuncEnd;
	}

	hwlog_info("%s interrupt 0x%04x\n", __func__, di->irq_val);
	idtp9221_clear_interrupt(di->irq_val);

	ret = idtp9221_get_chip_id(&chip_id);
	if (ret) {
		hwlog_err("%s: get chip_id fail\n", __func__);
		return;
	}
	if (chip_id == STWLC33_CHIP_ID) {
		ret = stwlc33_rx_ready_check_handler(di);
		if (ret)
			goto FuncEnd;
	}

	if (di->irq_val & IDT9221_RX_STATUS_READY) {
		di->irq_val &= ~IDT9221_RX_STATUS_READY;
		idtp9221_rx_ready_handler(di);
	}
	if (di->irq_val & IDT9221_RX_STATUS_POWER_ON) {
		di->irq_val &= ~IDT9221_RX_STATUS_POWER_ON;
		idtp9221_rx_power_on_handler(di);
	}
	if (di->irq_val & IDT9221_RX_STATUS_OCP) {
		di->irq_val &= ~IDT9221_RX_STATUS_OCP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OCP, NULL);
	}
	if (di->irq_val & IDT9221_RX_STATUS_OVP) {
		di->irq_val &= ~IDT9221_RX_STATUS_OVP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OVP, NULL);
	}
	if (di->irq_val & IDT9221_RX_STATUS_OTP) {
		di->irq_val &= ~IDT9221_RX_STATUS_OTP;
		stwlc33_get_rx_chip_temp();
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OTP, NULL);
	}
	/* receice data from TX, please handler the interrupt */
	if (di->irq_val & IDT9221_RX_STATUS_TXDATA_RECEIVED)
		idtp9221_data_received_handler(di);

FuncEnd:
	/* clear interrupt again */
	if (!gpio_get_value(di->gpio_int)) {
		idtp9221_read_word(IDT9221_RX_INT_STATUS_ADDR, &irq_value);
		hwlog_info("[%s] gpio_int is low, irq_value = 0x%x\n", __func__, irq_value);
		idtp9221_clear_interrupt(WORD_MASK);
	}
}

static void idtp9221_irq_work(struct work_struct *work)
{
	u8 mode = 0;
	struct idtp9221_device_info *di = container_of(work,
		struct idtp9221_device_info, irq_work);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		idtp9221_wake_unlock();
		return;
	}

	/* get System Operating Mode */
	idtp9221_get_mode(&mode);

	/* handler interrupt */
	if ((mode & IDT9221_TX_WPCMODE) || (mode & IDT9221_BACKPOWERED))
		idtp9221_tx_mode_irq_handler(di);
	else
		idtp9221_rx_mode_irq_handler(di);

	if (!irq_abnormal_flag)
		idtp9221_enable_irq(di);
	idtp9221_wake_unlock();
}

static irqreturn_t idtp9221_interrupt(int irq, void *_di)
{
	struct idtp9221_device_info *di = _di;

	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return IRQ_HANDLED;
	}

	idtp9221_wake_lock();
	hwlog_info("%s ++\n", __func__);
	if (di->irq_active == 1) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = 0;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("irq is not enable,do nothing\n");
		idtp9221_wake_unlock();
	}
	hwlog_info("%s --\n", __func__);

	return IRQ_HANDLED;
}

static void idtp9221_pmic_vbus_handler(bool vbus_state, void *dev_data)
{
	u16 irq_val;
	int ret;
	int vrect = 0;
	int wired_state;
	struct idtp9221_device_info *di = dev_data;

	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return;
	}
	wired_state = wlrx_get_wired_channel_state();
	if (vbus_state && irq_abnormal_flag && idtp9221_is_tx_exist(dev_data) &&
		(wired_state != WIRED_CHANNEL_ON)) {
		ret = idtp9221_read_word(IDT9221_RX_INT_STATUS_ADDR, &irq_val);
		if (ret) {
			hwlog_err("%s: read interrupt fail, clear interrupt\n", __func__);
			return;
		}
		(void)idtp9221_get_rx_vrect(&vrect, dev_data);
		hwlog_info("[%s] irq_val = 0x%x, vrect = %dmV\n",
			__func__, irq_val, vrect);
		if ((irq_val & IDT9221_RX_STATUS_READY) ||
			(vrect > STWLC33_VRECT_THRES_VAL)) {
			idtp9221_clear_interrupt(WORD_MASK);
			idtp9221_rx_ready_handler(di);
			di->irq_cnt = 0;
			irq_abnormal_flag = false;
			idtp9221_enable_irq(di);
		}
	}
}

static int idtp9221_parse_fod(struct device_node *np,
	struct idtp9221_device_info *di)
{
	if (!np || !di) {
		hwlog_err("%s: np or di null\n", __func__);
		return -EINVAL;
	}
	if (!g_support_idt_wlc)
		return 0;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_5v", di->rx_fod_5v, IDT9221_RX_FOD_COEF_LEN))
		return -EINVAL;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_9v", di->rx_fod_9v, IDT9221_RX_FOD_COEF_LEN))
		return -EINVAL;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_12v", di->rx_fod_12v, IDT9221_RX_FOD_COEF_LEN))
		return -EINVAL;

	return 0;
}

static int stwlc33_parse_fod(struct device_node *np,
	struct idtp9221_device_info *di)
{
	if (!np || !di) {
		hwlog_err("%s: np or di null\n", __func__);
		return -EINVAL;
	}
	if (!g_support_st_wlc)
		return 0;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"st_rx_fod_5v", di->st_rx_fod_5v, STWLC33_RX_FOD_COEF_LEN))
		return -EINVAL;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"st_rx_fod_9v", di->st_rx_fod_9v, STWLC33_RX_FOD_COEF_LEN))
		return -EINVAL;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"st_rx_fod_12v", di->st_rx_fod_12v, STWLC33_RX_FOD_COEF_LEN))
		return -EINVAL;

	return 0;
}

static int idtp9221_parse_dts(struct device_node *np, struct idtp9221_device_info *di)
{
	int ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", &di->rx_ss_good_lth, IDT9221_RX_SS_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", &di->gpio_en_valid_val, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_th_5v", &di->tx_fod_th_5v, IDT9221_TX_FOD_THD0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sc_rx_vrmax_gap", &di->sc_rx_vrmax_gap, 150); /* volt=gap*21/4095=0.769v */

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_idt_wlc", &g_support_idt_wlc, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_st_wlc", &g_support_st_wlc, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"i2c_trans_fail_limit", &di->i2c_trans_fail_limit, 0);

	ret = idtp9221_parse_fod(np, di);
	if (ret) {
		hwlog_err("%s: get idt fod para fail\n", __func__);
		return -EINVAL;
	}
	ret = stwlc33_parse_fod(np, di);
	if (ret) {
		hwlog_err("%s: get st fod para fail\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int idtp9221_gpio_init(struct idtp9221_device_info *di, struct device_node *np)
{
	if (power_gpio_config_output(np,
		"gpio_en", "idt9221_en",
		&di->gpio_en, di->gpio_en_valid_val))
		return -EINVAL;

	if (power_gpio_config_output(np,
		"gpio_sleep_en", "idt9221_sleep_en",
		&di->gpio_sleep_en, RX_SLEEP_EN_DISABLE)) {
		gpio_free(di->gpio_en);
		return -EINVAL;
	}

	return 0;
}

static int idtp9221_irq_init(struct idtp9221_device_info *di, struct device_node *np)
{
	int ret;

	if (power_gpio_config_interrupt(np,
		"gpio_int", "idt9221_int", &di->gpio_int, &di->irq_int)) {
		ret = -EINVAL;
		goto irq_init_fail_0;
	}

	ret = request_irq(di->irq_int, idtp9221_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "idt9221_irq", di);
	if (ret) {
		hwlog_err("could not request idt9221_irq\n");
		di->irq_int = -1;
		goto irq_init_fail_1;
	}
	enable_irq_wake(di->irq_int);
	di->irq_active = 1;
	INIT_WORK(&di->irq_work, idtp9221_irq_work);

	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return ret;
}

static struct wlrx_ic_ops g_idtp9221_rx_ic_ops = {
	.fw_update = idtp9221_rx_fwupdate,
	.chip_init = idt_st_chip_init,
	.chip_reset = idtp9221_chip_reset,
	.chip_enable = idtp9221_chip_enable,
	.is_chip_enable = idtp9221_is_chip_enable,
	.sleep_enable = idtp9221_sleep_enable,
	.is_sleep_enable = idtp9221_is_sleep_enable,
	.get_vrect = idtp9221_get_rx_vrect,
	.get_vout = idtp9221_get_rx_vout,
	.get_iout = idtp9221_get_rx_iout,
	.get_vout_reg = idtp9221_get_rx_vout_reg,
	.get_vfc_reg = idtp9221_get_vfc_reg,
	.set_vfc = idtp9221_set_vfc,
	.set_vout = idtp9221_set_rx_vout,
	.get_fop = idtp9221_get_rx_fop,
	.get_chip_info = idt_st_get_chip_info_str,
	.get_fod_coef = idtp9221_get_rx_fod_coef,
	.set_fod_coef = idtp9221_set_rx_fod_coef,
	.is_tx_exist = idtp9221_is_tx_exist,
	.kick_watchdog = idtp9221_kick_watchdog,
	.send_ept = idtp9221_send_ept,
	.stop_charging = idtp9221_stop_charging,
	.pmic_vbus_handler = idtp9221_pmic_vbus_handler,
	.read_nvm_info = idt_st_read_nvm,
	.get_bst_delay_time = idtp9221_get_bst_delay_time,
};

static struct wltx_ic_ops g_idtp9221_tx_ic_ops = {
	.fw_update = idtp9221_tx_fwupdate,
	.chip_init = idtp9221_tx_chip_init,
	.chip_reset = idtp9221_chip_reset,
	.chip_enable = idtp9221_chip_enable,
	.mode_enable = idtp9221_enable_tx_mode,
	.set_open_flag = idtp9221_set_tx_open_flag,
	.set_stop_cfg = idtp9221_tx_stop_config,
	.is_rx_discon = idtp9221_check_rx_disconnect,
	.is_in_tx_mode = idtp9221_in_tx_mode,
	.is_in_rx_mode = idtp9221_in_rx_mode,
	.get_vrect = idtp9221_get_tx_vrect,
	.get_vin = idtp9221_get_tx_vin,
	.get_iin = idtp9221_get_tx_iin,
	.get_temp = idtp9221_get_chip_temp,
	.get_fop = idtp9221_get_tx_fop,
	.get_ping_freq = idtp9221_get_tx_ping_freq,
	.get_ping_interval = idtp9221_get_tx_ping_interval,
	.get_min_fop = idtp9221_get_tx_min_fop,
	.get_max_fop = idtp9221_get_tx_max_fop,
	.set_ping_freq = idtp9221_set_tx_ping_freq,
	.set_ping_interval = idtp9221_set_tx_ping_interval,
	.set_min_fop = idtp9221_set_tx_min_fop,
	.set_max_fop = idtp9221_set_tx_max_fop,
};

static struct qi_protocol_ops idtp9221_qi_protocol_ops = {
	.chip_name = "idtp9221",
	.send_msg = idtp9221_send_msg,
	.send_msg_with_ack = idtp9221_send_msg_ack,
	.receive_msg = idtp9221_receive_msg,
	.send_fsk_msg = idtp9221_send_fsk_msg,
	.get_ask_packet = idtp9221_get_ask_packet,
};

static void idtp9221_shutdown(struct i2c_client *client)
{
	struct idtp9221_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	hwlog_info("%s ++\n", __func__);
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) {
		idtp9221_chip_reset(di);
		idtp9221_set_vfc(ADAPTER_5V * WL_MVOLT_PER_VOLT, di);
		idtp9221_set_rx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT, di);
		msleep(IDT9221_SHUTDOWN_SLEEP_TIME);
	}
	hwlog_info("%s --\n", __func__);
}

static int idtp9221_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	u16 chip_id = 0;
	struct idtp9221_device_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	g_idtp9221_di = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);
	INIT_WORK(&di->rx_ready_check_work, stwlc33_rx_ready_check_work);
	ret = idtp9221_parse_dts(np, di);
	if (ret)
		goto idt9221_fail_0;

	ret = idtp9221_gpio_init(di, np);
	if (ret)
		goto idt9221_fail_0;
	ret = idtp9221_irq_init(di, np);
	if (ret)
		goto idt9221_fail_1;

	wakeup_source_init(&g_idtp9221_wakelock, "idtp9221_wakelock");
	mutex_init(&di->mutex_irq);

	g_idtp9221_rx_ic_ops.dev_data = (void *)di;
	ret = wlrx_ic_ops_register(&g_idtp9221_rx_ic_ops, WLTRX_IC_MAIN);
	if (ret)
		goto idt9221_fail_2;

	g_idtp9221_tx_ic_ops.dev_data = (void *)di;
	ret = wltx_ic_ops_register(&g_idtp9221_tx_ic_ops, WLTRX_IC_MAIN);
	if (ret)
		goto idt9221_fail_2;

	if (idtp9221_is_tx_exist(di)) {
		stwlc33_need_rst_check();
		idtp9221_clear_interrupt(WORD_MASK);
		hwlog_info("[%s] rx exsit, execute rx_ready_handler\n", __func__);
		idtp9221_rx_ready_handler(di);
	} else {
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_ENABLE);
	}

	idtp9221_qi_protocol_ops.dev_data = (void *)g_idtp9221_di;
	qi_protocol_ops_register(&idtp9221_qi_protocol_ops, WLTRX_IC_MAIN);
	g_idtp9221_handle = qi_protocol_get_handle();

	idtp9221_get_chip_id(&chip_id);
	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = chip_id;
		power_dev_info->ver_id = 0;
	}

	hwlog_info("wireless_idtp9221 probe ok\n");
	return 0;

idt9221_fail_2:
	wakeup_source_trash(&g_idtp9221_wakelock);
	free_irq(di->irq_int, di);
idt9221_fail_1:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_sleep_en);
idt9221_fail_0:
	devm_kfree(&client->dev, di);
	di = NULL;
	np = NULL;
	return ret;
}
MODULE_DEVICE_TABLE(i2c, wireless_idtp9221);
static struct of_device_id idtp9221_of_match[] = {
	{
		.compatible = "huawei, wireless_idtp9221",
		.data = NULL,
	},
	{},
};
static const struct i2c_device_id idtp9221_i2c_id[] = {
	{ "wireless_idtp9221", 0 }, {}
};

static struct i2c_driver idtp9221_driver = {
	.probe = idtp9221_probe,
	.shutdown = idtp9221_shutdown,
	.id_table = idtp9221_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_idtp9221",
		.of_match_table = of_match_ptr(idtp9221_of_match),
	},
};

static int __init idtp9221_init(void)
{
	return i2c_add_driver(&idtp9221_driver);
}

static void __exit idtp9221_exit(void)
{
	i2c_del_driver(&idtp9221_driver);
}

fs_initcall_sync(idtp9221_init);
module_exit(idtp9221_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wpc  receiver module driver");
MODULE_AUTHOR("HUAWEI Inc");
