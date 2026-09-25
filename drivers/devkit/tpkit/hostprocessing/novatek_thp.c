#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include "huawei_thp.h"


#define NOVATECH_IC_NAME "novatech"
#define THP_NOVA_DEV_NODE_NAME "novatech"

#define SPI_WRITE_MASK(a)	(a | 0x80)
#define SPI_READ_MASK(a)	(a & 0x7F)

#define NVT_LOW_WORD_BIT_MASK(x) (uint8_t)((x >> 7) & 0x000000FF)
#define NVT_HI_WORD_BIT_MASK(x) (uint8_t)((x >> 15) & 0x000000FF)
#define NVT_ID_BYTE_MAX 6
#define NVT_SW_RESET_RW_LEN 4
#define NVT_SW_RESET_REG_ADDR 0x3F0FE
#define NVT_SW_RESET_DELAY_MS 10
#define NVT_SW_RETRY_TIME_DELAY_MS 10
#define NVT_CHIP_VER_TRIM_RW_LEN 16
#define NVT_CHIP_VER_TRIM_REG_ADDR 0x1F64E
#define NVT_CHIP_VER_TRIM_RETRY_TIME 5
#define WAIT_FOR_SPI_BUS_READ_DELAY 5

#define NVT_CMD_ADDR 0x21C50
#define NVT_CMD_STAMP_ADDR 0x21C77
#define NVT_GET_GESTURE_ADDR 0x218CC
#define NVT_CMD_ENTER_CUSTOMIZED_DP_DSTB 0x7D
#define NVT_CMD_ENTER_GESTURE_MODE 0x13
#define FORMAL_REPORT 1
#define RETRY_NUM 20
#define SEND_CMD_LEN 3
#define GESTURE_DATA_LEN 9
#define NOVA_DEBUG 1
#define D_SEND_CMD_RETEY 5
#define D_SEND_CMD_LEN 7
#define NVT_SPI_RW_BUF_LEN 10

struct nvt_ts_trim_id_table_entry {
	uint8_t id[NVT_ID_BYTE_MAX];
	uint8_t mask[NVT_ID_BYTE_MAX];
};

static const struct nvt_ts_trim_id_table_entry trim_id_table[] = {
	{.id = {0x0C, 0xFF, 0xFF, 0x72, 0x66, 0x03}, .mask = {1, 0, 0, 1, 1, 1} },
	{.id = {0x0C, 0xFF, 0xFF, 0x82, 0x66, 0x03}, .mask = {1, 0, 0, 1, 1, 1} },
	{.id = {0x0A, 0xFF, 0xFF, 0x72, 0x65, 0x03}, .mask = {1, 0, 0, 1, 1, 1} },
	{.id = {0x0B, 0xFF, 0xFF, 0x25, 0x65, 0x03}, .mask = {1, 0, 0, 1, 1, 1} },
	{.id = {0x0A, 0xFF, 0xFF, 0x72, 0x67, 0x03}, .mask = {1, 0, 0, 1, 1, 1} },
	{.id = {0x0A, 0xFF, 0xFF, 0x82, 0x66, 0x03}, .mask = {1, 0, 0, 1, 1, 1} },
	{.id = {0x0B, 0xFF, 0xFF, 0x82, 0x66, 0x03}, .mask = {1, 0, 0, 1, 1, 1} },
	{.id = {0x0A, 0xFF, 0xFF, 0x72, 0x66, 0x03}, .mask = {1, 0, 0, 1, 1, 1} },
	{.id = {0x55, 0x00, 0xFF, 0x00, 0x00, 0x00}, .mask = {1, 1, 0, 1, 1, 1} },
	{.id = {0x55, 0x72, 0xFF, 0x00, 0x00, 0x00}, .mask = {1, 1, 0, 1, 1, 1} },
	{.id = {0xAA, 0x00, 0xFF, 0x00, 0x00, 0x00}, .mask = {1, 1, 0, 1, 1, 1} },
	{.id = {0xAA, 0x72, 0xFF, 0x00, 0x00, 0x00}, .mask = {1, 1, 0, 1, 1, 1} },
	{.id = {0xFF, 0xFF, 0xFF, 0x72, 0x67, 0x03}, .mask = {0, 0, 0, 1, 1, 1} },
	{.id = {0xFF, 0xFF, 0xFF, 0x70, 0x66, 0x03}, .mask = {0, 0, 0, 1, 1, 1} },
	{.id = {0xFF, 0xFF, 0xFF, 0x70, 0x67, 0x03}, .mask = {0, 0, 0, 1, 1, 1} },
	{.id = {0xFF, 0xFF, 0xFF, 0x72, 0x66, 0x03}, .mask = {0, 0, 0, 1, 1, 1} },
	{.id = {0xFF, 0xFF, 0xFF, 0x25, 0x65, 0x03}, .mask = {0, 0, 0, 1, 1, 1} },
	{.id = {0xFF, 0xFF, 0xFF, 0x70, 0x68, 0x03}, .mask = {0, 0, 0, 1, 1, 1} },
	{.id = {0xFF, 0xFF, 0xFF, 0x26, 0x65, 0x03}, .mask = {0, 0, 0, 1, 1, 1} }
};

struct nvt_ts_spi_buf {
	uint8_t *w_buf;
	uint8_t *r_buf;
} g_nvt_ts_spi_buf;

static int touch_driver_spi_read_write(struct spi_device *client,
	void *tx_buf, void *rx_buf, size_t len)
{
	struct spi_transfer t = {
		.tx_buf = tx_buf,
		.rx_buf = rx_buf,
		.len    = len,
	};
	struct spi_message m;
	int rc;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	rc = thp_bus_lock();
	if (rc < 0) {
		THP_LOG_ERR("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
#ifndef CONFIG_HUAWEI_THP_MTK
	thp_spi_cs_set(GPIO_HIGH);
#endif
	rc = thp_spi_sync(client, &m);
	thp_bus_unlock();

	return rc;
}

static int touch_driver_check_spi_master_capacity(struct spi_device *sdev)
{
	if (sdev->master->flags & SPI_MASTER_HALF_DUPLEX) {
		THP_LOG_ERR("%s: Full duplex not supported by master\n", __func__);
		return -EIO;
	} else {
		return 0;
	}
}

void touch_driver_sw_reset_idle(struct spi_device *sdev)
{
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	/*This register addr is for NT36870 */
	uint32_t addr = NVT_SW_RESET_REG_ADDR;
	unsigned int rw_len;

	if (!w_buf) {
		THP_LOG_ERR("%s: w_buf is null\n", __func__);
		return;
	}
	/* write 0xAA @0x3F0FE */
	w_buf[0] = SPI_WRITE_MASK(0x7F);
	w_buf[1] = NVT_HI_WORD_BIT_MASK(addr);
	w_buf[2] = NVT_LOW_WORD_BIT_MASK(addr);
	rw_len = 3;

	touch_driver_spi_read_write(sdev, w_buf, NULL, rw_len);

	w_buf[0] = (uint8_t)(SPI_WRITE_MASK((addr & 0x0000007F)));
	w_buf[1] = 0xAA;
	rw_len = 2;
	touch_driver_spi_read_write(sdev, w_buf, NULL, rw_len);

	thp_time_delay(NVT_SW_RESET_DELAY_MS);
}

void touch_driver_bootloader_reset(struct spi_device *sdev)
{
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	/*This register addr is for NT36870 */
	uint32_t addr = NVT_SW_RESET_REG_ADDR;
	unsigned int rw_len;

	THP_LOG_INFO("%s: called\n", __func__);
	if (!w_buf) {
		THP_LOG_ERR("%s: w_buf is null\n", __func__);
		return;
	}
	/* write 0xAA @0x3F0FE */
	w_buf[0] = SPI_WRITE_MASK(0x7F);
	w_buf[1] = NVT_HI_WORD_BIT_MASK(addr);
	w_buf[2] = NVT_LOW_WORD_BIT_MASK(addr);
	rw_len = 3;

	touch_driver_spi_read_write(sdev, w_buf, NULL, rw_len);

	w_buf[0] = (uint8_t)(SPI_WRITE_MASK((addr & 0x0000007F)));
	w_buf[1] = 0x69;/* bootloader reset cmd */
	rw_len = 2;
	touch_driver_spi_read_write(sdev, w_buf, NULL, rw_len);

	thp_time_delay(NVT_SW_RESET_DELAY_MS);
}

static int touch_driver_check_ver_trim_table(uint8_t *r_buf)
{
	int i;
	int list;

	THP_LOG_INFO("%s:begin\n", __func__);

	for (list = 0; list < ARRAY_SIZE(trim_id_table); list++) {
		for (i = 0; i < NVT_ID_BYTE_MAX; i++) {
			if (trim_id_table[list].mask[i] &&
				r_buf[i + 2] != trim_id_table[list].id[i])
				break;
		}

		if (i == NVT_ID_BYTE_MAX) {
			THP_LOG_INFO("%s:done, list=%d\n", __func__, list);
			return 0;
		}
	}

	return -ENODEV;
}

static void touch_driver_timing_work(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;

	THP_LOG_ERR("%s:called,do sw reset idle in hard reset\n", __func__);
	cd = tdev->thp_core;
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_time_delay(tdev->timing_config.boot_reset_low_delay_ms);
	touch_driver_sw_reset_idle(tdev->sdev);
	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
#else
	if (tdev->thp_core->support_pinctrl == 0) {
		THP_LOG_INFO("%s: not support pinctrl\n", __func__);
		return;
	}
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_low);
	thp_time_delay(tdev->timing_config.boot_reset_low_delay_ms);
	touch_driver_sw_reset_idle(tdev->sdev);
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_high);
	thp_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
#endif
}

static int8_t touch_driver_check_chip_ver_trim(struct spi_device *sdev)
{
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	uint8_t *r_buf = g_nvt_ts_spi_buf.r_buf;
	uint32_t addr = NVT_CHIP_VER_TRIM_REG_ADDR;
	int rw_len;
	int32_t retry;

	if (!w_buf) {
		THP_LOG_ERR("%s: w_buf is null\n", __func__);
		return -EINVAL;
	}
	if (!r_buf) {
		THP_LOG_ERR("%s: r_buf is null\n", __func__);
		return -EINVAL;
	}
	/* Check for 5 times */
	for (retry = NVT_CHIP_VER_TRIM_RETRY_TIME; retry > 0; retry--) {
		touch_driver_bootloader_reset(sdev);
		touch_driver_sw_reset_idle(sdev);
		/* read chip id @0x1F64E */
		w_buf[0] = SPI_WRITE_MASK(0x7F);
		w_buf[1] = NVT_HI_WORD_BIT_MASK(addr);
		w_buf[2] = NVT_LOW_WORD_BIT_MASK(addr);
		rw_len = 3;
		touch_driver_spi_read_write(sdev, w_buf, NULL, rw_len);

		memset(w_buf, 0, NVT_CHIP_VER_TRIM_RW_LEN);
		w_buf[0] = (uint8_t)(SPI_READ_MASK(addr));
		rw_len = 8;
		touch_driver_spi_read_write(sdev, w_buf, r_buf, rw_len);

		if (!touch_driver_check_ver_trim_table(r_buf))
			return 0;

		thp_time_delay(NVT_SW_RETRY_TIME_DELAY_MS);
	}

	return -ENODEV;
}

static int touch_driver_init(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = tdev->thp_core;
	struct device_node *nova_node = of_get_child_by_name(cd->thp_node,
						THP_NOVA_DEV_NODE_NAME);

	THP_LOG_INFO("%s: called\n", __func__);

	if (!nova_node) {
		THP_LOG_INFO("%s: dev not config in dts\n", __func__);
		return -ENODEV;
	}

	rc = touch_driver_check_spi_master_capacity(tdev->sdev);
	if (rc) {
		THP_LOG_ERR("%s: spi capacity check fail\n", __func__);
		return rc;
	}

	rc = thp_parse_spi_config(nova_node, cd);
	if (rc)
		THP_LOG_ERR("%s: spi config parse fail\n", __func__);

	rc = thp_parse_timing_config(nova_node, &tdev->timing_config);
	if (rc)
		THP_LOG_ERR("%s: timing config parse fail\n", __func__);

	return 0;
}

static int touch_driver_chip_detect(struct thp_device *tdev)
{

	int rc;

	g_nvt_ts_spi_buf.w_buf = kzalloc(NVT_CHIP_VER_TRIM_RW_LEN, GFP_KERNEL);
	if (!g_nvt_ts_spi_buf.w_buf) {
		THP_LOG_ERR("%s:w_buf request memory fail\n", __func__);
		rc = -ENOMEM;
		goto exit;
	}

	g_nvt_ts_spi_buf.r_buf = kzalloc(NVT_CHIP_VER_TRIM_RW_LEN, GFP_KERNEL);
	if (!g_nvt_ts_spi_buf.r_buf) {
		THP_LOG_ERR("%s:r_buf request memory fail\n", __func__);
		rc = -ENOMEM;
		goto exit;
	}

	touch_driver_timing_work(tdev);

	rc =  touch_driver_check_chip_ver_trim(tdev->thp_core->sdev);
	if (rc) {
		THP_LOG_INFO("%s:chip is not identified\n", __func__);
		rc = -ENODEV;
		goto exit;
	}

	return 0;

exit:
	kfree(g_nvt_ts_spi_buf.w_buf);
	g_nvt_ts_spi_buf.w_buf = NULL;
	kfree(g_nvt_ts_spi_buf.r_buf);
	g_nvt_ts_spi_buf.r_buf = NULL;
	if (tdev->thp_core->fast_booting_solution) {
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}
	return rc;
}
static int touch_driver_get_frame(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	unsigned char get_frame_cmd = 0x20; /* read frame data command */
	uint8_t *w_buf = NULL;

	if (!tdev) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}

	if (!len) {
		THP_LOG_INFO("%s: read len illegal\n", __func__);
		return -ENOMEM;
	}

	w_buf = tdev->tx_buff;

	memset(tdev->tx_buff, 0, THP_MAX_FRAME_SIZE);
	/* write cmd 0x20 and get frame */
	w_buf[0] = (uint8_t)(SPI_READ_MASK(get_frame_cmd));
	touch_driver_spi_read_write(tdev->thp_core->sdev, w_buf, buf, len);

	return 0;
}

static int touch_driver_resume(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;

	THP_LOG_INFO("%s: called\n", __func__);
	cd = tdev->thp_core;
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_set_value(tdev->gpios->cs_gpio, 1);
	touch_driver_sw_reset_idle(tdev->sdev);
	gpio_set_value(tdev->gpios->rst_gpio, 1);
	thp_time_delay(tdev->timing_config.resume_reset_after_delay_ms);
#else
	if (tdev->thp_core->support_pinctrl == 0) {
		THP_LOG_INFO("%s: not support pinctrl\n", __func__);
		return 0;
	}
	pinctrl_select_state(tdev->thp_core->pctrl, cd->mtk_pinctrl.cs_high);
	touch_driver_sw_reset_idle(tdev->sdev);
	pinctrl_select_state(tdev->thp_core->pctrl, cd->mtk_pinctrl.reset_high);
	thp_time_delay(tdev->timing_config.resume_reset_after_delay_ms);
#endif
	return 0;
}

static int touch_driver_wrong_touch(struct thp_device *tdev)
{
	if ((!tdev) || (!tdev->thp_core)) {
		THP_LOG_ERR("%s: input dev is null\n", __func__);
		return -EINVAL;
	}

	if (tdev->thp_core->support_gesture_mode) {
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		tdev->thp_core->easy_wakeup_info.off_motion_on = true;
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		THP_LOG_INFO("%s, done\n", __func__);
	}
	return 0;
}

static int touch_driver_gesture_report(struct thp_device *tdev,
	unsigned int *gesture_wakeup_value)
{
	uint8_t *r_buf = g_nvt_ts_spi_buf.r_buf;
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	int retval;
	int i;

	if ((!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev)) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	if ((gesture_wakeup_value == NULL) ||
		(!tdev->thp_core->support_gesture_mode)) {
		THP_LOG_ERR("%s, gesture not support\n");
		return -EINVAL;
	}
	if ((r_buf == NULL) || (w_buf == NULL)) {
		THP_LOG_INFO("%s: r_buf or w_buf is null\n", __func__);
		return -ENOMEM;
	}

	memset(r_buf, 0, NVT_SPI_RW_BUF_LEN);
	memset(w_buf, 0, NVT_SPI_RW_BUF_LEN);
	w_buf[0] = SPI_WRITE_MASK(0x7F);
	w_buf[1] = NVT_HI_WORD_BIT_MASK(NVT_GET_GESTURE_ADDR);
	w_buf[2] = NVT_LOW_WORD_BIT_MASK(NVT_GET_GESTURE_ADDR);
	for (i = 0; i < RETRY_NUM; i++) {
		retval = touch_driver_spi_read_write(tdev->thp_core->sdev,
			w_buf, NULL, SEND_CMD_LEN);
		if (retval == 0)
			break;
		THP_LOG_INFO("%s: spi write abnormal retval %d retry\n",
			__func__, retval);
		msleep(WAIT_FOR_SPI_BUS_READ_DELAY); /* retry time delay */
	}

	memset(r_buf, 0, NVT_SPI_RW_BUF_LEN);
	memset(w_buf, 0, NVT_SPI_RW_BUF_LEN);
	w_buf[0] = (uint8_t)(SPI_READ_MASK(NVT_GET_GESTURE_ADDR));
	retval = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, r_buf, GESTURE_DATA_LEN);
	if (retval < 0)
		THP_LOG_ERR("%s: spi read abnormal retval %d\n",
			__func__, retval);
	/* calc r_buf2 and r_buf3 to check data is valid */
	if (((~r_buf[2] + 1) & 0xFF) != r_buf[3])
		return -EINVAL;
	THP_LOG_INFO("%s, gesture trigger\n", __func__);
	mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
	if (tdev->thp_core->easy_wakeup_info.off_motion_on == true) {
		tdev->thp_core->easy_wakeup_info.off_motion_on = false;
		*gesture_wakeup_value = TS_DOUBLE_CLICK;
	}
	mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);

	return 0;
}

static int touch_driver_read_fw_stamp(struct thp_device *tdev,
	uint8_t *stamp_val)
{
	uint8_t *r_buf = g_nvt_ts_spi_buf.r_buf;
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	int rw_len;
	int ret;

	if ((r_buf == NULL) || (w_buf == NULL)) {
		THP_LOG_INFO("%s: r_buf or w_buf is null\n", __func__);
		return -ENOMEM;
	}
	memset(r_buf, 0, NVT_SPI_RW_BUF_LEN);
	memset(w_buf, 0, NVT_SPI_RW_BUF_LEN);
	/* Read FW cmd stamp */
	w_buf[0] = SPI_WRITE_MASK(0x7F);
	w_buf[1] = NVT_HI_WORD_BIT_MASK(NVT_CMD_STAMP_ADDR);
	w_buf[2] = NVT_LOW_WORD_BIT_MASK(NVT_CMD_STAMP_ADDR);
	rw_len = SEND_CMD_LEN;
	ret = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, NULL, rw_len);
	if (ret) {
		THP_LOG_ERR("%s, send write stamp cmd failed\n", __func__);
		return ret;
	}

	memset(w_buf, 0, SEND_CMD_LEN);
	memset(r_buf, 0, SEND_CMD_LEN);
	w_buf[0] = (uint8_t)(SPI_READ_MASK((NVT_CMD_STAMP_ADDR & 0x0000007F)));
	rw_len = SEND_CMD_LEN;
	ret = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, r_buf, rw_len);
	if (ret) {
		THP_LOG_ERR("%s, send read stamp cmd failed\n", __func__);
		return ret;
	}

	THP_LOG_INFO("%s current stamp is %x %x [%x]\n", __func__,
		r_buf[0], r_buf[1], r_buf[2]);
	*stamp_val = r_buf[2]; /* byte2 is valid value */

	return NO_ERR;
}

static int touch_driver_write_cmd_addr(struct thp_device *tdev)
{
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	int rw_len;
	int ret;

	if (w_buf == NULL) {
		THP_LOG_INFO("%s: w_buf is null\n", __func__);
		return -ENOMEM;
	}
	/* write cmd @0x21C50 */
	memset(w_buf, 0, NVT_SPI_RW_BUF_LEN);
	w_buf[0] = SPI_WRITE_MASK(0x7F);
	w_buf[1] = NVT_HI_WORD_BIT_MASK(NVT_CMD_ADDR);
	w_buf[2] = NVT_LOW_WORD_BIT_MASK(NVT_CMD_ADDR);
	rw_len = SEND_CMD_LEN;

	ret = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, NULL, rw_len);
	if (ret) {
		THP_LOG_ERR("%s, send write 0x21C50 failed\n", __func__);
		return ret;
	}
	return NO_ERR;
}

static int touch_driver_send_fw_cmd(struct thp_device *tdev, uint8_t cmd)
{
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	int rw_len;
	uint8_t cur_idx;
	uint8_t loop;
	uint8_t stamp_val;
	int ret;

	if (w_buf == NULL) {
		THP_LOG_INFO("%s: w_buf is null\n", __func__);
		return -ENOMEM;
	}
	ret = touch_driver_read_fw_stamp(tdev, &stamp_val);
	if (ret) {
		THP_LOG_ERR("%s, fail read fw stamp\n");
		return ret;
	}
	THP_LOG_INFO("read stamp value is %x, cmd is %x\n", stamp_val, cmd);
	ret = touch_driver_write_cmd_addr(tdev);
	if (ret)
		return ret;
	/*
	 * 0:r/w
	 * 1:cmd
	 * 2:handshake
	 * 3/4:arg
	 * 5:stamp, use previous number in case collision with HAL
	 * 6:checksum
	 */
	memset(w_buf, 0, NVT_SPI_RW_BUF_LEN);
	w_buf[0] = (uint8_t)(SPI_WRITE_MASK((NVT_CMD_ADDR & 0x0000007F)));
	w_buf[1] = cmd;
	w_buf[2] = 0xBB;
	w_buf[3] = 0x00;
	w_buf[4] = 0x00;
	w_buf[5] = stamp_val - 1;
	w_buf[6] = (uint8_t)(~(w_buf[1] + w_buf[2] + w_buf[3] +
		w_buf[4] + w_buf[5]) + 1);
	rw_len = D_SEND_CMD_LEN;
	ret = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, NULL, rw_len);
	if (ret < 0) {
		THP_LOG_ERR("%s, send write %u failed\n", __func__, cmd);
		return ret;
	}

	/* Check if cmd was received and wait handle 50ms */
	cur_idx = w_buf[5];
	stamp_val = 0;
	for (loop = 0; loop < D_SEND_CMD_RETEY; loop++) {
		mdelay(1); /* wait fw respone, retry delay time */
		ret = touch_driver_read_fw_stamp(tdev, &stamp_val);
		if (ret) {
			THP_LOG_ERR("%s, fail read fw stamp\n");
			return ret;
		}
		if (stamp_val == cur_idx) {
			THP_LOG_INFO("%s cmd successfully received\n",
				__func__);
			/* spec, make sure gesture enter success */
			msleep(50);
			return NO_ERR;
		}
	}
#if defined(CONFIG_HUAWEI_DSM)
	thp_dmd_report(DSM_TPHOSTPROCESSING_DEV_GESTURE_EXP1,
		"%s, stamp_val is 0x%x, cur_idx is 0x%x\n", __func__,
		stamp_val, cur_idx);
#endif
	THP_LOG_INFO("%s cmd failed!\n", __func__);
	return -EINVAL;
}

static void touch_driver_enter_gesture_mode(
	struct thp_device *tdev)
{
	int retval;

	retval = touch_driver_send_fw_cmd(tdev,
		NVT_CMD_ENTER_GESTURE_MODE);
	if (retval < 0)
		THP_LOG_ERR("%s, enter gesture mode failed\n", __func__);
	mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
	tdev->thp_core->easy_wakeup_info.off_motion_on = true;
	mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
}

static int touch_driver_suspend(struct thp_device *tdev)
{
	int pt_test_mode;
	struct thp_core_data *cd = NULL;
	enum ts_sleep_mode gesture_status;

	THP_LOG_INFO("%s: called\n", __func__);
	cd = tdev->thp_core;
	if ((!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev)) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}

	pt_test_mode = is_pt_test_mode(tdev);
	gesture_status = tdev->thp_core->easy_wakeup_info.sleep_mode;

	if (tdev->thp_core->support_gesture_mode &&
		tdev->thp_core->lcd_gesture_mode_support) {
		if (pt_test_mode) { /* PT test spec */
			THP_LOG_INFO("%s: PT sleep mode\n", __func__);
			/* gesture mode don't need */
			(void)touch_driver_send_fw_cmd(tdev,
				NVT_CMD_ENTER_CUSTOMIZED_DP_DSTB);
			gpio_set_value(tdev->gpios->cs_gpio, 0);
		} else if (gesture_status == TS_GESTURE_MODE &&
			tdev->thp_core->lcd_gesture_mode_support) {
			THP_LOG_INFO("%s: TS_GESTURE_MODE\n", __func__);
			touch_driver_enter_gesture_mode(tdev);
		} else { /* power off spec */
			THP_LOG_INFO("%s: power off mode\n", __func__);
			/* gesture mode don't need */
			(void)touch_driver_send_fw_cmd(tdev,
				NVT_CMD_ENTER_CUSTOMIZED_DP_DSTB);
			gpio_set_value(tdev->gpios->rst_gpio, 0);
			gpio_set_value(tdev->gpios->cs_gpio, 0);
		}
		thp_time_delay(
			tdev->timing_config.suspend_reset_after_delay_ms);
		return 0;
	}

#ifndef CONFIG_HUAWEI_THP_MTK
	if (pt_test_mode) {
		THP_LOG_INFO("%s: sleep mode\n", __func__);
		gpio_set_value(tdev->gpios->cs_gpio, 0);
	} else {
		THP_LOG_INFO("%s: power off mode\n", __func__);
		gpio_set_value(tdev->gpios->rst_gpio, 0);
		gpio_set_value(tdev->gpios->cs_gpio, 0);
	}
#else
	if (cd->support_pinctrl == 0) {
		THP_LOG_INFO("%s: not support pinctrl\n", __func__);
		return 0;
	}

	if (pt_test_mode) {
		THP_LOG_INFO("%s: sleep mode\n", __func__);
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_low);
	} else {
		THP_LOG_INFO("%s: power off mode\n", __func__);
		pinctrl_select_state(cd->pctrl, cd->pins_idle);
	}
#endif

	thp_time_delay(tdev->timing_config.suspend_reset_after_delay_ms);

	return 0;
}

static void touch_driver_exit(struct thp_device *tdev)
{
	THP_LOG_INFO("%s: called\n", __func__);
	kfree(g_nvt_ts_spi_buf.w_buf);
	g_nvt_ts_spi_buf.w_buf = NULL;
	kfree(g_nvt_ts_spi_buf.r_buf);
	g_nvt_ts_spi_buf.r_buf = NULL;
	kfree(tdev->tx_buff);
	kfree(tdev->rx_buff);
	kfree(tdev);
}

struct thp_device_ops nova_dev_ops = {
	.init = touch_driver_init,
	.detect = touch_driver_chip_detect,
	.get_frame = touch_driver_get_frame,
	.resume = touch_driver_resume,
	.suspend = touch_driver_suspend,
	.exit = touch_driver_exit,
	.chip_wrong_touch = touch_driver_wrong_touch,
	.chip_gesture_report = touch_driver_gesture_report,
};

static int __init touch_driver_module_init(void)
{
	int rc;
	struct thp_core_data *cd = thp_get_core_data();

	struct thp_device *dev = kzalloc(sizeof(struct thp_device), GFP_KERNEL);
	if (!dev) {
		THP_LOG_ERR("%s: thp device malloc fail\n", __func__);
		return -ENOMEM;
	}

	dev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	dev->rx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!dev->tx_buff || !dev->rx_buff) {
		THP_LOG_ERR("%s: out of memory\n", __func__);
		rc = -ENOMEM;
		goto err;
	}

	dev->ic_name = NOVATECH_IC_NAME;
	dev->ops = &nova_dev_ops;
	if (cd && cd->fast_booting_solution) {
		thp_send_detect_cmd(dev, NO_SYNC_TIMEOUT);
		/*
		 * The thp_register_dev will be called later to complete
		 * the real detect action.If it fails, the detect function will
		 * release the resources requested here.
		 */
		return 0;
	}
	rc = thp_register_dev(dev);
	if (rc) {
		THP_LOG_ERR("%s: register fail\n", __func__);
		goto err;
	}

	return rc;
err:
	if(dev->tx_buff){
		kfree(dev->tx_buff);
		dev->tx_buff = NULL;
	}
	if(dev->rx_buff){
		kfree(dev->rx_buff);
		dev->rx_buff = NULL;
	}
	if(dev){
		kfree(dev);
		dev = NULL;
	}
	return rc;
}
static void __exit touch_driver_module_exit(void)
{
	THP_LOG_INFO("%s: called \n", __func__);
};

module_init(touch_driver_module_init);
module_exit(touch_driver_module_exit);
MODULE_AUTHOR("novatech");
MODULE_DESCRIPTION("novatech driver");
MODULE_LICENSE("GPL");
