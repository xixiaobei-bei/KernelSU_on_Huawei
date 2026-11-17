/*
 * Copyright (C) 2016 ST Microelectronics S.A.
 * Copyright (C) 2010 Stollmann E+V GmbH
 * Copyright (C) 2010 Trusted Logic S.A.
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <st21nfc.h>
#include <linux/of_gpio.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/clk.h>

#include <linux/pm_wakeup.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include "securec.h"
#include <linux/mtd/hisi_nve_interface.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#define SET_PMU_REG_BIT(reg_val, bit_pos) ((reg_val) |= 1<<(bit_pos))
#define CLR_PMU_REG_BIT(reg_val, bit_pos) ((reg_val) &= ~(1<<(bit_pos)))

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

#ifdef CONFIG_EXT_INPUTHUB
#include "ext_sensorhub_api.h"
#endif

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/nfc/nfc_interface.h>
#define HWLOG_TAG nfc
HWLOG_REGIST();
#define NFC_TRY_NUM 3
#define MAX_TIMES 10
//#include <huawei_platform/log/hw_log.h>
//#define HWLOG_TAG nfc

#define MAX_BUFFER_SIZE 260

#define DRIVER_VERSION "2.0.0"

#define CHAR_0 48
#define CHAR_9 57
#define RSSI_MAX 65535
#define NFC_NCI_HEAD_TYPE 0x7E
#define DATA_HEAD_LEN 4


/* define the active state of the WAKEUP pin */
#define ST21_IRQ_ACTIVE_HIGH 1
#define ST21_IRQ_ACTIVE_LOW 0

#define MAX_NFC_CHIP_TYPE_SIZE		32

#define ST_NFC_DMD_PROBE_TIMER (10 * HZ)  /* delay to probe dmd */
#ifdef CONFIG_EXT_INPUTHUB
#define NFC_SID_CID_COUNT     1 /* watch serviceid commandid */
#define MAX_TRY_SET_VEN_MCU   15 /* max time watch set ven to mcu */
#endif
static int g_nfc_ext_gpio; /* use extented gpio, eg.codec */
static int g_nfc_svdd_sw; /* use for svdd switch */
static int g_nfc_ese_type = NFC_WITHOUT_ESE; /* record ese type wired to nfcc by dts */
static char g_nfc_chip_type[MAX_NFC_CHIP_TYPE_SIZE];
static int g_nfc_watch_type = NFC_WITHOUT_WATCH_TYPE; /* watch type use for control extented gpio */
static int g_is4bytesheader = 1;
static int g_spi_cs_gpio;
static struct timer_list st_nfc_dmd_probe_timer;
#ifdef CONFIG_EXT_INPUTHUB
static int g_nfc_setven_status = 0; /* read mcu ven status */

static struct sid_cid g_nfc_read_mcu_sid_cid[NFC_SID_CID_COUNT] = {
	{ MC_NFC_WATCH_WALLET_SVR, MC_NFC_WATCH_WALLET_SET_VEN_RESET_CMD },
};
static struct subscribe_cmds g_nfc_read_mcu_cmds = {
	g_nfc_read_mcu_sid_cid, NFC_SID_CID_COUNT
};
#endif

/* prototypes */
static irqreturn_t st21nfc_dev_irq_handler(int irq, void *dev_id);
/*
 * The platform data member 'polarity_mode' defines
 * how the wakeup pin is configured and handled.
 * it can take the following values :
 *	 IRQF_TRIGGER_RISING
 *   IRQF_TRIGGER_FALLING
 *   IRQF_TRIGGER_HIGH
 *   IRQF_TRIGGER_LOW
 */

static void get_ext_gpio_type(void);
static void nfc_gpio_set_value(unsigned int gpio, int val);
static int nfc_get_dts_config_string(const char *node_name,
				const char *prop_name,
				char *out_string,
				int out_string_len);
static int nfc_gpio_get_value(unsigned int gpio);
static void get_wake_lock_timeout(void);

struct st21nfc_platform {
	struct mutex read_mutex;
	struct i2c_client *client;
	unsigned int irq_gpio;
	unsigned int reset_gpio;
	unsigned int ena_gpio;
	unsigned int polarity_mode;
	unsigned int active_polarity;	/* either 0 (low-active) or 1 (high-active)  */
};

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_dev st_dsm_nfc = {
	.name = "st_dsm_nfc",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};
static struct dsm_client *nfc_dclient = NULL;

#endif

static bool irqIsAttached;

static bool device_open; /* Is device open? */
static void get_nfc_wired_ese_type(void);
static void get_nfc_watch_type(void);

#ifdef CONFIG_EXT_INPUTHUB
static int register_watch_mcu_nfc_callback(void);
#endif

/*0 -- hisi cpu(default); 1 -- hisi pmu*/
static int g_nfc_clk_src = NFC_CLK_SRC_CPU;
static int g_nfcservice_lock;
static int g_nfc_on_type; /* nfc ven controlled by which type of gpio: gpio/hisi_pmic/regulator_bulk*/

static int g_nfc_single_channel;
static int g_nfc_card_num;

static int nfc_at_result;
static int nfc_switch_state;
static int g_nfc_close_type;
static int firmware_update;
static int g_nfc_spec_rf;
static char g_nfc_nxp_config_name[MAX_CONFIG_NAME_SIZE];
static char g_nfc_st_rf_config_name[MAX_CONFIG_NAME_SIZE];
static char g_nfc_brcm_conf_name[MAX_CONFIG_NAME_SIZE];
static int g_nfc_ese_num;
static char g_nfc_fw_version[MAX_NFC_FW_VERSION_SIZE];
static int g_nfcservice_lock;
static int g_nfc_activated_se_info; /* record activated se info when nfc enable process */
static int g_nfc_hal_dmd_no;	/* record last hal dmd no */
static long nfc_get_rssi = 0;


/*0 -- close nfcservice normally; 1 -- close nfcservice with enable CE */

static int g_clk_status_flag = 0;/* use for judging whether clk has been enabled */

static t_pmu_reg_control nfc_on_hi6421v600 = {0x240, 0};
static t_pmu_reg_control nfc_on_hi6421v500 = {0x0C3, 4};
static t_pmu_reg_control nfc_on_hi6555v110 = {0x158, 0};
static t_pmu_reg_control nfc_on_hi6421v700 = {0x2E6, 0};
static t_pmu_reg_control nfc_on_hi6421v800 = {0x2E6, 0};
static t_pmu_reg_control nfc_on_hi6555v300 = {0x22D, 0};
static t_pmu_reg_control nfc_on_hi6555v500 = {0x2E5, 0};


static struct wakeup_source wlock_read;
static int g_wake_lock_timeout = WAKE_LOCK_TIMEOUT_DISABLE;

struct st21nfc_dev {
	wait_queue_head_t read_wq;
	struct miscdevice st21nfc_device;
	bool irq_enabled;
	struct st21nfc_platform platform_data;
	spinlock_t irq_enabled_lock;
	struct clk *nfc_clk;
	struct pinctrl *pctrl;
	struct pinctrl_state *pins_default;
	struct pinctrl_state *pins_idle;
    int sim_switch;
	int sim_status;
	int enable_status;
};

static int nfc_nv_opera(int opr, int idx, const char *name, int len, void *data)
{
	int ret = -1;
	struct hisi_nve_info_user user_info;
	hwlog_info("%s: enter.\n", __func__);

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = opr;
	user_info.nv_number = idx;
	user_info.valid_size = len;

	strncpy(user_info.nv_name, name, sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';

	/* write data to nv */
	if (opr == NV_WRITE)
		memcpy((char *)user_info.nv_data, (char *)data, len);

	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		pr_err("%s:nve_direct_access read error(%d)\n", __func__, ret);
		return -1;
	}
	memcpy((char *)data, (char *)user_info.nv_data, len);
	return 0;
}


static void st21nfc_disable_irq(struct st21nfc_dev *st21nfc_dev)
{
	unsigned long flags;
    hwlog_info("%s : Enter IRQ %d\n", __func__, st21nfc_dev->platform_data.client->irq);
	spin_lock_irqsave(&st21nfc_dev->irq_enabled_lock, flags);
	if (st21nfc_dev->irq_enabled) {
		disable_irq_nosync(st21nfc_dev->platform_data.client->irq);
		st21nfc_dev->irq_enabled = false;
	}
	spin_unlock_irqrestore(&st21nfc_dev->irq_enabled_lock, flags);
	hwlog_info("%s : exit , irq_enable=%d\n", __func__,st21nfc_dev->irq_enabled);
}

static void print_hex_string(int bytes_read, const char *buf, int len)
{
	int ret = 0;
	int i;
	// 1 hex byte match 2 characters, and add end symbol '\0'
	int buffer_len = bytes_read * 2 + 1;
	char *print_str = NULL;
	if (bytes_read > MAX_BUFFER_SIZE || bytes_read <= 0 ||
		bytes_read > len || buf == NULL) {
		hwlog_err("%s:param error \n", __func__);
		return;
	}
	print_str = (char *)kzalloc(buffer_len, GFP_KERNEL);
	if (print_str == NULL) {
		hwlog_err("%s:Cannot allocate memory for read tmpStr.\n",
			__func__);
		return;
	}
	(void)memset_s(print_str, buffer_len, 0x0, buffer_len);
	for (i = 0; i < bytes_read; i++) {
		// 1 hex byte match 2 characters
		ret = snprintf_s(&print_str[i * 2], buffer_len - i * 2, 2,
			"%02X", buf[i]);
		if (ret < 0) {
			hwlog_err("%s:snprintf_s fail, i=%d.\n", __func__, i);
			break;
		}
	}
	if (ret > 0)
		hwlog_info("%s RX: %s\n", __func__, print_str);

	kfree(print_str);
}

static ssize_t st_i2c_read(struct  st21nfc_dev *pdev, char *buf, int count)
{
	int ret = 0;
	int pinlev;
	bool isSuccess = false;
	int size = 0;
	int retry = 0;
	int hdrsz = g_is4bytesheader ? 4 : 3;
	int extra = 0;
	int bytes_read;

	if (count < DATA_HEAD_LEN || pdev == NULL || buf == NULL) {
		hwlog_info("%s:input param error\n", __func__);
		return -1;
	} else if (count > MAX_BUFFER_SIZE) {
		count = MAX_BUFFER_SIZE;
		hwlog_info("%s:count set max\n", __func__);
	}
	mutex_lock(&pdev->platform_data.read_mutex);
	pinlev = nfc_gpio_get_value(pdev->platform_data.irq_gpio);//nfc_gpio_get_value instead of gpio_get_value

	hwlog_info("%s: pinlev=0x%x\n", __func__, pinlev);

	/* Read data, we have 3 chances */
	do {
		bytes_read = i2c_master_recv(pdev->platform_data.client,
			buf, hdrsz);
		hwlog_info("%s : retry = %d, bytes_read = %d\n", __func__,
			retry, bytes_read);
		print_hex_string(bytes_read, buf, count);
		if (bytes_read == (int)hdrsz) {
			// 0x6000:nci cmd head
			if (hdrsz == DATA_HEAD_LEN &&
				buf[0] == 0x60 && buf[1] == 0x00)
				// read only 3 bytes until next Reset
				// We are either in loader mode or in older fw
				g_is4bytesheader = 0;
		}
		// 0x7E:st21nfc rsp data first byte
		if (hdrsz == DATA_HEAD_LEN && buf[0] != NFC_NCI_HEAD_TYPE) {
			// we read 1 payload byte already
			extra = 1;
		} else if (hdrsz == DATA_HEAD_LEN) {
			// we got dummy 7e,discard and continue as if not there
			// shift nci head byte[1][2][3] to heard byte[0][1][2]
			buf[0] = buf[1];
			buf[1] = buf[2];
			buf[2] = buf[3];
		}
		if (buf[0] != NFC_NCI_HEAD_TYPE &&
			buf[1] != NFC_NCI_HEAD_TYPE) {
			isSuccess = true;
		} else {
			// we got some dummy bytes read (on top of extra byte)
			// shift nci head byte[1][2] to heard byte[0][1]
			if (buf[1] != NFC_NCI_HEAD_TYPE) {
				buf[0] = buf[1];
				buf[1] = buf[2];
				// read 1 byte data to buf[3]
				bytes_read = i2c_master_recv(
					pdev->platform_data.client, buf + 2, 1);
				if (bytes_read == 1)
					isSuccess = true;
			} else if (buf[2] != NFC_NCI_HEAD_TYPE) {
				buf[0] = buf[2];
				// read 2 bytes data to buf[1]buf[2]
				bytes_read = i2c_master_recv(
					pdev->platform_data.client, buf + 1, 2);
				if (bytes_read == 2)
					isSuccess = true;
			} else {
				// there was no data ready, wait 3ms
				msleep(3);
			}
		}
		if (isSuccess == true) {
			// buf[2]: payload length
			int remaining = buf[2];
			bytes_read = 0;
			// read remaining data,3 + extra:
			// 3 bytes nci head and length of extra payload
			if (remaining - extra > 0)
				bytes_read = i2c_master_recv(
					pdev->platform_data.client,
					buf + 3 + extra,
					remaining - extra);

			if (bytes_read == remaining - extra)
				// 3: nci head length
				size = 3 + extra + bytes_read;
			else
				isSuccess = false;
		}
		retry++;
		// 0x6F02: firmware log, skip this log
		if ((buf[0] == 0x6F && buf[1] == 0x02) && isSuccess == true) {
			isSuccess = false;
			retry = 0;
			print_hex_string(size, buf, count);
		}
	} while(isSuccess == false && retry < MAX_TIMES);

	print_hex_string(size, buf, count);
	if (false == isSuccess) {
		hwlog_err("%s : i2c_master_recv returned %d, line = %d\n", __func__, bytes_read, __LINE__);
		ret = -EIO;
	}
	mutex_unlock(&pdev->platform_data.read_mutex);
	return ret;
}


static ssize_t st_i2c_write(struct  st21nfc_dev *pdev, const char *buf, int count)
{
	char tmp[MAX_BUFFER_SIZE];
	int ret = count;
	char *tmpStr = NULL;
	int retry;
	int i;
	bool isSuccess = false;

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;
/*
	if (copy_from_user(tmp, buf, count)) {
		hwlog_err("%s : failed to copy from user space\n", __func__);
		return -EFAULT;
	}
*/
	tmpStr = (char *)kzalloc(sizeof(tmp)*2 + 1, GFP_KERNEL);
	if (!tmpStr) {
		hwlog_info("%s:Cannot allocate memory for write tmpStr.\n", __func__);
		return -ENOMEM;
	}
	/* Write data */
	for (retry = 0; retry < NFC_TRY_NUM; retry++) {
	    ret = i2c_master_send(pdev->platform_data.client, buf, count);
		for (i = 0; i < count; i++) {
			snprintf(&tmpStr[i * 2], 3, "%02X", buf[i]);
		}
		hwlog_info("%s : retry = %d, ret = %d, count = %3d > %s\n", __func__, retry, ret, (int)count, tmpStr);
		if (ret == (int)count) {
			isSuccess = true;
			break;
		}else {
			if (retry > 0) {
				hwlog_info("%s : send data try =%d returned %d\n", __func__, retry, ret);
			}
			msleep(1);
			continue;
		}
	}
	kfree(tmpStr);
	if (false == isSuccess) {
		hwlog_err("%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}
	return ret;
}


static ssize_t nfc_get_rssi_store(struct device *dev, struct device_attribute *attr,
            const char *buf, size_t count)
{
    int i = 0;
    int flag = 1;
    nfc_get_rssi = 0;
    //pr_info("%s:%s,%d, %d\n", __func__, buf, nfc_get_rssi, count);
    if(buf!=NULL)
    {
        if (buf[0] == '-')
        {
            flag = -1;
            i++;
        }
        while (buf[i] != '\0')
        {
            //hwlog_info("%s:%s,%d,%d, %d\n", __func__, buf,i, nfc_get_rssi, count);
            if((buf[i] >= CHAR_0) && (buf[i] <= CHAR_9) && (nfc_get_rssi <= RSSI_MAX))
            {
                nfc_get_rssi=(long)(nfc_get_rssi*10) + (buf[i]-CHAR_0); /*file storage str*/
            }
            i++;
        }
        nfc_get_rssi = flag * nfc_get_rssi;
    }
    //pr_info("%s:%s,%d, %d\n", __func__, buf, nfc_get_rssi, count);
    return (ssize_t)count;
}

static ssize_t nfc_get_rssi_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    pr_info("%s:%s,%ld\n", __func__, buf, nfc_get_rssi);
    return (ssize_t)(snprintf(buf, 256, "%ld", nfc_get_rssi));
}


static ssize_t nfc_recovery_close_nfc_store(struct device *dev, struct device_attribute *attr,
            const char *buf, size_t count)
{
    return (ssize_t)count;
}

static int recovery_close_nfc(struct i2c_client *client, struct  st21nfc_dev *pdev)
{
	int ret;
	int nfc_rece_length = 256;
	unsigned char recv_buf[256] = {0};
	const  char core_reset_cmd[] = {0x20, 0x00, 0x01, 0x00};
	const  char init_cmd[] = {0x20, 0x01, 0x02, 0x00, 0x00};
	const  char set_ven_config[] = {0x2f,0x02,0x02,0x02,0x00};
	unsigned int fw_version = 0;

	/*reset*/
	pr_info("%s Double Pulse Request,reset_gpio=%d\n", __func__,pdev->platform_data.reset_gpio);
    if (pdev->platform_data.reset_gpio != 0) {
        /* pulse low for 20 millisecs */
        nfc_gpio_set_value(pdev->platform_data.reset_gpio,
                0);
        pr_info("%s,reset_gpio=%d\n", __func__,nfc_gpio_get_value(pdev->platform_data.reset_gpio));
        msleep(20);
        nfc_gpio_set_value(pdev->platform_data.reset_gpio,
                1);
        msleep(10);
        /* pulse low for 20 millisecs */
        nfc_gpio_set_value(pdev->platform_data.reset_gpio,
                0);
        msleep(20);
        nfc_gpio_set_value(pdev->platform_data.reset_gpio,
                1);

        pr_info("%s done Double Pulse Request,reset_gpio=%d\n", __func__,nfc_gpio_get_value(pdev->platform_data.reset_gpio));
    }
	g_is4bytesheader = 1;
	msleep(10);

	/* read response */
	ret = st_i2c_read(pdev, recv_buf, nfc_rece_length);
	if ((ret < 0) || !((recv_buf[0] == 0x60) && (recv_buf[1] == 0x00))) {
		pr_err("%s: reset failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	fw_version = (recv_buf[10] << 24) | (recv_buf[11] << 16)
		| (recv_buf[12] << 8) | recv_buf[13];
	pr_info("%s: fw_version, 0x%x\n", __func__, fw_version);
	if (fw_version == 0) {
		pr_err("%s: no firmware inside, nothing to do.\n", __func__);
		goto failed;
	}
	/* send core reset command */
	ret = st_i2c_write(pdev, core_reset_cmd, sizeof(core_reset_cmd));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/* read response */
	ret = st_i2c_read(pdev, recv_buf, nfc_rece_length);
	if ((ret < 0) || !(((recv_buf[0] == 0x40) && (recv_buf[1] == 0x00)) || ((recv_buf[0] == 0x60) && (recv_buf[1] == 0x00)))) {
		pr_err("%s: st_i2c_read core reset failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/* read response */
	ret = st_i2c_read(pdev, recv_buf, nfc_rece_length);
	if ((ret < 0) || !(((recv_buf[0] == 0x40) && (recv_buf[1] == 0x00)) || ((recv_buf[0] == 0x60) && (recv_buf[1] == 0x00)))) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/* write CORE_INIT_CMD */
	ret = st_i2c_write(pdev, init_cmd, sizeof(init_cmd));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/* read response */
	ret = st_i2c_read(pdev, recv_buf, nfc_rece_length);
	if ((ret < 0) || !(((recv_buf[0] == 0x40) && (recv_buf[1] == 0x01)) || ((recv_buf[0] == 0x60) && (recv_buf[1] == 0x06) ))) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	ret = st_i2c_read(pdev, recv_buf, nfc_rece_length);
	if ((ret < 0) || !(((recv_buf[0] == 0x40) && (recv_buf[1] == 0x01)) || ((recv_buf[0] == 0x60) && (recv_buf[1] == 0x06) ))) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	msleep(2);

	/* write set_ven_config */
	ret = st_i2c_write(pdev, set_ven_config, sizeof(set_ven_config));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/*read response*/
	ret = st_i2c_read(pdev, recv_buf, nfc_rece_length);
	if ((ret < 0) || !((recv_buf[0] == 0x4F) && (recv_buf[1] == 0x02) && (recv_buf[3] == 0x00))) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	msleep(2);
	ret = st_i2c_read(pdev, recv_buf, nfc_rece_length);
	if ((ret < 0) || !((recv_buf[0] == 0x60) && (recv_buf[1] == 0x00))) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	return 0;
failed:
	return -1;
}

static int nfc_check_enable(struct i2c_client *client, struct  st21nfc_dev *pdev)
{
	int ret;
	int nfc_rece_length = 50; /* 50 for i2c read len */
	unsigned char recv_buf[100] = {0};
	if (g_nfc_watch_type > 0) {
		pr_info("%s watch reset_gpio\n", __func__);
		return g_nfc_watch_type;
	}
	/*reset*/
	pr_info("%s Double Pulse Request,reset_gpio=%d\n", __func__,pdev->platform_data.reset_gpio);
	if (pdev->platform_data.reset_gpio != 0) {
		/* pulse low for 20 millisecs */
		nfc_gpio_set_value(pdev->platform_data.reset_gpio, 0);
		pr_info("%s,reset_gpio=%d\n", __func__,nfc_gpio_get_value(pdev->platform_data.reset_gpio));
		msleep(20);
		nfc_gpio_set_value(pdev->platform_data.reset_gpio, 1);
		msleep(10);
		/* pulse low for 20 millisecs */
		nfc_gpio_set_value(pdev->platform_data.reset_gpio, 0);
		msleep(20);
		nfc_gpio_set_value(pdev->platform_data.reset_gpio, 1);
		pr_info("%s done Double Pulse Request,reset_gpio=%d\n", __func__,nfc_gpio_get_value(pdev->platform_data.reset_gpio));
	}
	msleep(20);

	/*read response*/
	ret = st_i2c_read(pdev, recv_buf, nfc_rece_length);
	return ret;
}

/* hw_modify for fw update */
int st_set_fwupdate_on_info(char info)
{
	int ret;
	char hex_para[TEL_HUAWEI_NV_NFCCAL_LEGTH] = {0};
	hwlog_info("%s: enter!\n", __func__);
	/* read nfc configure value from nvm */
	ret = nfc_nv_opera(NV_READ, TEL_HUAWEI_NV_NFCCAL_NUMBER, TEL_HUAWEI_NV_NFCCAL_NAME, TEL_HUAWEI_NV_NFCCAL_LEGTH, hex_para);
	if (ret < 0) {
		hwlog_err("%s: get nv error ret: %d!\n", __func__, ret);
		return -1;
	}
	/* write nfc configure value to nvm */
	hex_para[FWUPDATE_ON_OFFSET] = info;
	ret = nfc_nv_opera(NV_WRITE, TEL_HUAWEI_NV_NFCCAL_NUMBER, TEL_HUAWEI_NV_NFCCAL_NAME, TEL_HUAWEI_NV_NFCCAL_LEGTH, hex_para);
	if (ret < 0) {
		hwlog_err("%s: set nv error ret: %d!\n", __func__, ret);
		return -1;
	}
	return 0;
}

static ssize_t nfc_recovery_close_nfc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int status = -1;
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct st21nfc_dev *st21nfc_dev = NULL;

	/* write fwUpdate_on to nvm when recovery */
	hwlog_info("%s: enter.\n", __func__);
	(void)st_set_fwupdate_on_info(1);

	st21nfc_dev = i2c_get_clientdata(i2c_client_dev);
	if (st21nfc_dev == NULL) {
		pr_err("%s: st21nfc_dev == NULL!\n", __func__);
		return 0;
	}
	pr_info("%s: enter!\n", __func__);
	status = recovery_close_nfc(i2c_client_dev, st21nfc_dev);
	if (status < 0) {
		pr_err("%s: recovery_close_nfc error!\n", __func__);
	}
	status = 0; // add to avoid nfc fail add by HW
	pr_info("%s: status=%d\n", __func__, status);
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d\n", status));
}


static ssize_t nfc_clk_src_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d", g_nfc_clk_src));
}


static ssize_t nfc_calibration_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char cali_info[TEL_HUAWEI_NV_NFCCAL_LEGTH] = {0};
	char cali_str[MAX_BUFFER_SIZE] = {0};
	int ret = -1, i = 0;
	int cal_len = 0;
	ret = nfc_nv_opera(NV_READ, TEL_HUAWEI_NV_NFCCAL_NUMBER, (char*)TEL_HUAWEI_NV_NFCCAL_NAME, TEL_HUAWEI_NV_NFCCAL_LEGTH, (void*)cali_info);
	if (ret < 0) {
		pr_err("%s: get nv error ret: %d!\n", __func__, ret);
		return -1;
	}
	cal_len = cali_info[0];
	for (i = 0; i < cal_len + 1; i ++) {
		snprintf(&cali_str[i*2], 3, "%02X", cali_info[i]);
	}
	pr_info("%s: nfc cal info: [%s]!\n", __func__, cali_str);
	return (ssize_t)(snprintf(buf, MAX_BUFFER_SIZE-1, "%s", cali_str));
}


int st_nfc_record_dmd_info(long dmd_no, const char *dmd_info)
{
/*lint -e529 -esym(529,*)*/
#ifdef CONFIG_HUAWEI_DSM
	if (dmd_no < NFC_DMD_NUMBER_MIN || dmd_no > NFC_DMD_NUMBER_MAX
		|| dmd_info == NULL || NULL == nfc_dclient) {
		pr_info("%s: para error: %ld\n", __func__, dmd_no); /*lint !e960*/
		return -1;
	}

	pr_info("%s: dmd no: %ld - %s", __func__, dmd_no, dmd_info); /*lint !e960*/
	if (!dsm_client_ocuppy(nfc_dclient)) {
		dsm_client_record(nfc_dclient, "DMD info:%s", dmd_info);
		dsm_client_notify(nfc_dclient, dmd_no);
	}
#endif
	return 0;
}

static void st_nfc_record_dmd_probe_timerhandler(unsigned long data)
{
	del_timer(&st_nfc_dmd_probe_timer);

	if (get_nfc_dmd_probe_flag() == 0)
		st_nfc_record_dmd_info(NFC_DMD_I2C_READ_ERROR_NO, "ST21NFC-Error-in-Probe-Error");
}

static ssize_t nfc_hal_dmd_info_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	long val = 0;
	char dmd_info_from_hal[64] = {'\0'};
	/* The length of DMD error number is 9. */
	if (sscanf(buf, "%9ld", &val) == 1) {
		if (val < NFC_DMD_NUMBER_MIN || val > NFC_DMD_NUMBER_MAX) {
			return (ssize_t)count;
		}
		g_nfc_hal_dmd_no = val;
		/* The max length of content for current dmd description set as 63.
		   Example for DMD Buf: '923002014 CoreReset:600006A000D1A72000'.
		   A space as a separator is between dmd error no and description.*/
		if (sscanf(buf, "%*s%63s", dmd_info_from_hal) == 1) {
			st_nfc_record_dmd_info(val, dmd_info_from_hal);
		}

	} else {
		pr_err("%s: get dmd number error\n", __func__);
	}
	return (ssize_t)count;
}

static ssize_t nfc_hal_dmd_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d", g_nfc_hal_dmd_no));
}

static ssize_t nfc_activated_se_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d", g_nfc_activated_se_info));
}

static ssize_t nfc_activated_se_info_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int val = 0;
	if (sscanf(buf, "%1d", &val) == 1) {
		g_nfc_activated_se_info = val;
	} else {
		pr_err("%s: set g_nfc_activated_se_info error\n", __func__);
		g_nfc_activated_se_info = 0;
	}
	pr_info("%s: g_nfc_activated_se_info:%d\n", __func__, g_nfc_activated_se_info);
	return (ssize_t)count;
}


static ssize_t nfcservice_lock_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d", g_nfcservice_lock));
}

static ssize_t nfcservice_lock_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int val = 0;

	if (sscanf(buf, "%1d", &val) == 1) {
		g_nfcservice_lock = val;
	} else {
		pr_err("%s: set g_nfcservice_lock error\n", __func__);
		g_nfcservice_lock = 0;
	}
	pr_info("%s: g_nfcservice_lock:%d\n", __func__, g_nfcservice_lock);
	return (ssize_t)count;
}

static ssize_t nfc_fw_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = -1;

	memset(g_nfc_fw_version, 0, MAX_NFC_FW_VERSION_SIZE);
	ret = nfc_get_dts_config_string("nfc_fw_version", "nfc_fw_version",
		g_nfc_fw_version, sizeof(g_nfc_fw_version));

	if (ret != 0) {
		memset(g_nfc_fw_version, 0, MAX_NFC_FW_VERSION_SIZE);
		pr_err("%s: can't get nfc g_nfc_fw_version, default FW 8.1.24\n", __func__);
		strcpy(g_nfc_fw_version, "FW 8.1.24");
	}
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%s", g_nfc_fw_version));
}

static int nfc_get_dts_config_u32(const char *node_name, const char *prop_name, u32 *pvalue)
{
	struct device_node *np = NULL;
	int ret = -1;

	for_each_node_with_property(np, node_name) {
		ret = of_property_read_u32(np, prop_name, pvalue);
		if (ret != 0) {
			pr_debug("%s: can not get prop values with prop_name: %s\n", __func__, prop_name);
		} else {
			pr_info("%s: %s=%d\n", __func__, prop_name, *pvalue);
		}
	}
	return ret;
}


void st_set_nfc_ese_num(void)
{
    int ret = -1;

    ret = nfc_get_dts_config_u32("st_nfc_ese_num", "st_nfc_ese_num", &g_nfc_ese_num);
    if (ret != 0) {
        g_nfc_ese_num = 1;
        pr_err("%s: can't get nfc ese num config!\n", __func__);
    }

    return;
}

static ssize_t nfc_ese_num_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d\n", (unsigned char)g_nfc_ese_num));
}


void st_set_nfc_nxp_config_name(void)
{
	int ret = -1;

	memset(g_nfc_nxp_config_name, 0, MAX_CONFIG_NAME_SIZE);
	ret = nfc_get_dts_config_string("st_nfc_nxp_name", "st_nfc_nxp_name",
					g_nfc_nxp_config_name, sizeof(g_nfc_nxp_config_name));

	if (ret != 0) {
		memset(g_nfc_nxp_config_name, 0, MAX_CONFIG_NAME_SIZE);
		pr_err("%s: can't get nfc nxp config name\n", __func__);
		return;
	}
	pr_info("%s: nfc nxp config name:%s\n", __func__, g_nfc_nxp_config_name);
	return;
}

void st_set_nfc_spec_rf(void)
{
	int ret;
	char spec_rf_dts_str[MAX_CONFIG_NAME_SIZE] = {0};

	memset(spec_rf_dts_str, 0, MAX_CONFIG_NAME_SIZE);
	ret = nfc_get_dts_config_string("nfc_spec_rf", "nfc_spec_rf",
		spec_rf_dts_str, sizeof(spec_rf_dts_str));

	if (ret != 0) {
		memset(spec_rf_dts_str, 0, MAX_CONFIG_NAME_SIZE);
		pr_err("%s: can't get nfc spec rf dts config\n", __func__);
		g_nfc_spec_rf = 0;	  // special rf config flag 0:normal
		return;
	}
	if (!strcasecmp(spec_rf_dts_str, "true"))
		g_nfc_spec_rf = 1;	  // special rf config flag 1:special

	pr_info("%s: nfc spec rf:%d\n", __func__, g_nfc_spec_rf);
	return;
}

void st_set_nfc_st_rf_config_name(void)
{
	int ret;

	memset(g_nfc_st_rf_config_name, 0, MAX_CONFIG_NAME_SIZE);
	ret = nfc_get_dts_config_string("nfc_st_rf_spec", "nfc_st_rf_spec",
		g_nfc_st_rf_config_name, sizeof(g_nfc_st_rf_config_name));

	if (ret != 0) {
		memset(g_nfc_st_rf_config_name, 0, MAX_CONFIG_NAME_SIZE);
		pr_err("%s: can't get nfc st rf spec config name\n", __func__);
		return;
	}
	pr_info("%s: nfc st rf spec config name:%s\n", __func__, g_nfc_st_rf_config_name);
	return;
}

void st_set_nfc_brcm_config_name(void)
{
	int ret = -1;

	memset(g_nfc_brcm_conf_name, 0, MAX_CONFIG_NAME_SIZE);
	ret = nfc_get_dts_config_string("st_nfc_brcm_conf_name", "st_nfc_brcm_conf_name",
					g_nfc_brcm_conf_name, sizeof(g_nfc_brcm_conf_name));

	if (ret != 0) {
		memset(g_nfc_brcm_conf_name, 0, MAX_CONFIG_NAME_SIZE);
		pr_err("%s: can't get nfc brcm config name\n", __func__);
		return;
	}
	pr_info("%s: nfc brcm config name:%s\n", __func__, g_nfc_brcm_conf_name);
	return;
}

static int check_sim_status(struct i2c_client *client, struct  st21nfc_dev *pdev)
{
    int ret;
	int nfc_rece_length = 200;
    int i = 0;

	unsigned char recvBuf[200] = {0};
    const  char init_cmd[] = {0x20, 0x01, 0x02, 0x00, 0x00};
    const  char set_mode_on[] = {0x2F, 0x02, 0x02, 0x02, 0x01};
    const  char core_init[] = {0x20, 0x01, 0x02, 0x00, 0x00};
    const  char hci_network[] = {0x01, 0x00, 0x02, 0x81, 0x03};
	const  char ese_cmd[]  = {0x01, 0x00, 0x04, 0x81, 0x01, 0x03, 0xC0};
	const  char send_cmd[] = {0x22, 0x00, 0x00};
	const  char ese_acti_cmd[] = {0x22, 0x01, 0x02, 0x82, 0x01};
	const  char ese_deac_cmd[] = {0x22, 0x01, 0x02, 0x82, 0x00};
    const  char sim_act_cmd[] = {0x22, 0x01, 0x02, 0x81, 0x01};
    const  char sim_deact_cmd[] = {0x22, 0x01, 0x02, 0x81, 0x00};

    /*reset*/
	pr_info("%s Double Pulse Request,reset_gpio=%d\n", __func__,pdev->platform_data.reset_gpio);
    if (pdev->platform_data.reset_gpio != 0) {
        /* pulse low for 20 millisecs */
        nfc_gpio_set_value(pdev->platform_data.reset_gpio,
                0);
        pr_info("%s,reset_gpio=%d\n", __func__,nfc_gpio_get_value(pdev->platform_data.reset_gpio));
        msleep(20);
        nfc_gpio_set_value(pdev->platform_data.reset_gpio,
                1);
        msleep(10);
        /* pulse low for 20 millisecs */
        nfc_gpio_set_value(pdev->platform_data.reset_gpio,
                0);
        msleep(20);
        nfc_gpio_set_value(pdev->platform_data.reset_gpio,
                1);

        pr_info("%s done Double Pulse Request,reset_gpio=%d\n", __func__,nfc_gpio_get_value(pdev->platform_data.reset_gpio));
    }
	msleep(10);

	/*core init*/

	/*read response*/
	/*write test swp cmd*/
	/*read response*/

	pdev->sim_status = 0;

    /*read response*/
	ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
	if ((ret < 0) || !((recvBuf[0] == 0x60) && (recvBuf[1] == 0x00))) {
		pr_err("%s: reset failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	/*write CORE_INIT_CMD*/
	ret = st_i2c_write(pdev, init_cmd, sizeof(init_cmd));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	/*read response*/
	ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
	if ((ret < 0) || !((recvBuf[0] == 0x40) && (recvBuf[1] == 0x01) && (recvBuf[3] == 0x00))) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	udelay(500);

	/*write Set mode ON*/
	ret = st_i2c_write(pdev, set_mode_on, sizeof(set_mode_on));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	/*read response*/
	ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
	if ((ret < 0) || !((recvBuf[0] == 0x4F) && (recvBuf[1] == 0x02) && (recvBuf[3] == 0x00))) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	udelay(1000);

	/*write CORE_INIT_CMD*/
	ret = st_i2c_write(pdev, core_init, sizeof(core_init));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	/*read response*/
	ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
	if (ret < 0) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	udelay(1000);

	/*write Open HCI network*/
	ret = st_i2c_write(pdev, hci_network, sizeof(hci_network));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	/*read response*/
	ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
	if (ret < 0) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	udelay(1000);

	/*write Set C0 (== SE HCI ID)*/
	ret = st_i2c_write(pdev, ese_cmd, sizeof(ese_cmd));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	/*read response*/
	ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
	if (ret < 0) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	udelay(1000);

	/*write send_CMD*/
	ret = st_i2c_write(pdev, send_cmd, sizeof(send_cmd));
	if (ret < 0) {
		pr_err("%s: st_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	/*read response*/
	ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
	if ((ret < 0) || !((recvBuf[0] == 0x42) && (recvBuf[1] == 0x00) && (recvBuf[3] == 0x00))) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	udelay(1000);

	/*write acti cmd*/
	ret = st_i2c_write(pdev, ese_acti_cmd, sizeof(ese_acti_cmd));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	/*read response*/
	ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
	if (ret < 0) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	for (i = 0; i < nfc_rece_length; i++) {
		// 0x62 and 0x00 is ese keyword of rsp, 3 is offset of 0x00
		if (recvBuf[i] == 0x62 && (i + 3) < sizeof(recvBuf)) {
			if (recvBuf[i + 3] == 0x00) {
				pr_err("%s: eSE_SUPPORT_CARD_EMULATION", __func__);
				pdev->sim_status |= eSE_SUPPORT_CARD_EMULATION;
			}
		}
	}

    mdelay(10);
	/*write deac cmd*/
	ret = st_i2c_write(pdev, ese_deac_cmd, sizeof(ese_deac_cmd));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/*read response*/
	ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
	if (ret < 0) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}

    mdelay(10);

    /*write deac cmd*/
	ret = st_i2c_write(pdev, sim_act_cmd, sizeof(sim_act_cmd));
	if (ret < 0) {
		pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}

    /*read response*/
	ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
	if (ret < 0) {
		pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	for (i = 0; i < nfc_rece_length; i++) {
		// 0x62 and 0x00 is ese keyword of rsp, 3 is offset of 0x00
		if (recvBuf[i] == 0x62 && (i + 3) < sizeof(recvBuf)) {
			if (recvBuf[i + 3] == 0x00) {
				pr_err("%s: UICC_SUPPORT_CARD_EMULATION", __func__);
				pdev->sim_status |= UICC_SUPPORT_CARD_EMULATION;
			}
		}
	}

    mdelay(5);

   /*write deac cmd*/
   ret = st_i2c_write(pdev, sim_deact_cmd, sizeof(sim_deact_cmd));
   if (ret < 0) {
       pr_err("%s: st_i2c_write failed, ret:%d\n", __func__, ret);
       goto failed;
   }
   /*read response*/
   ret = st_i2c_read(pdev, recvBuf, nfc_rece_length);
   if (ret < 0) {
       pr_err("%s: st_i2c_read core init failed, ret:%d\n", __func__, ret);
       goto failed;
   }

	return pdev->sim_status;
failed:
	pdev->sim_status = ret;
	return ret;
}


static ssize_t nfc_sim_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int status = -1;
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct st21nfc_dev *st21nfc_dev = NULL;

	if (g_nfc_watch_type > 0) {
		pr_info("%s watch have no sim!\n", __func__);
		return g_nfc_watch_type;
	}

	st21nfc_dev = i2c_get_clientdata(i2c_client_dev);
	if (st21nfc_dev == NULL) {
		pr_err("%s: st21nfc_dev == NULL!\n", __func__);
		return status;
	}
	pr_info("%s: enter!\n", __func__);
	status = check_sim_status(i2c_client_dev, st21nfc_dev);
	if (status < 0) {
		pr_err("%s: check_sim_status error!\n", __func__);
	}
	pr_info("%s: status=%d\n", __func__, status);
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d\n", status));
}


static ssize_t nfc_brcm_conf_name_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	return (ssize_t)count;
}
static ssize_t nfc_brcm_conf_name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, strlen(g_nfc_brcm_conf_name)+1, "%s", g_nfc_brcm_conf_name));
}

static ssize_t nfc_spec_rf_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%d",
		g_nfc_spec_rf));
}

static ssize_t nxp_config_name_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return (ssize_t)count;
}

static ssize_t nxp_config_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, strlen(g_nfc_nxp_config_name)+1, "%s",
		g_nfc_nxp_config_name));
}

static ssize_t st_rf_config_name_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	return (ssize_t)count;
}

static ssize_t st_rf_config_name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, strlen(g_nfc_st_rf_config_name) + 1, "%s", g_nfc_st_rf_config_name));
}

static ssize_t nfc_wired_ese_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d", g_nfc_ese_type));
}

static ssize_t nfc_wired_ese_info_store(struct device *dev, struct device_attribute *attr,
             const char *buf, size_t count)
{
        int val = 0;
        if (sscanf(buf, "%1d", &val) == 1) {
               g_nfc_ese_type = val;
        } else {
               pr_err("%s: set g_nfc_ese_type  error\n", __func__);
        }
        pr_err("%s: g_nfc_ese_type:%d\n", __func__,g_nfc_ese_type);
        return (ssize_t)count;
}


static ssize_t nfc_sim_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct st21nfc_dev *st21nfc_dev = NULL;
	st21nfc_dev = i2c_get_clientdata(i2c_client_dev);
	if (st21nfc_dev == NULL) {
		pr_err("%s: st21nfc_dev == NULL!\n", __func__);
		return 0;
	}
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d\n", st21nfc_dev->sim_switch));
}

static ssize_t nfc_sim_switch_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct st21nfc_dev *st21nfc_dev = NULL;
	int val = 0;

	st21nfc_dev = i2c_get_clientdata(i2c_client_dev);
	if (st21nfc_dev == NULL) {
		pr_err("%s: st21nfc_dev == NULL!\n", __func__);
		return 0;
	}
	if (sscanf(buf, "%1d", &val) == 1) {
		if (val >= 1 && val <= 3) {
			st21nfc_dev->sim_switch = val;
		} else {
			return -EINVAL;
		}
	} else
		return -EINVAL;
	return (ssize_t)count;
}

static ssize_t nfc_at_result_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	if(buf!=NULL)
	{
		nfc_at_result=buf[0]-CHAR_0; /*file storage str*/
	}
	return (ssize_t)count;
}

static ssize_t nfc_at_result_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, sizeof(nfc_at_result)+1, "%d", nfc_at_result));
}

static ssize_t nfc_single_channel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d", g_nfc_single_channel));
}

static ssize_t nfc_switch_state_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	if(buf!=NULL)
	{
		nfc_switch_state=buf[0]-CHAR_0; /*file storage str*/
	}
	return (ssize_t)count;
}

static ssize_t nfc_switch_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, sizeof(nfc_switch_state)+1, "%d", nfc_switch_state));
}

static ssize_t nfc_close_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d", g_nfc_close_type));
}

static ssize_t nfc_close_type_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int val = 0;
	if (sscanf(buf, "%1d", &val) == 1) {
		g_nfc_close_type = val;
	} else {
		pr_err("%s: set g_nfc_close_type error\n", __func__);
		g_nfc_close_type = 0;
	}
	pr_info("%s: g_nfc_close_type:%d\n", __func__, g_nfc_close_type);
	return (ssize_t)count;
}

int st_get_fwupdate_on_info(char *info)
{
	int ret;
	char hex_para[TEL_HUAWEI_NV_NFCCAL_LEGTH] = {0};
	hwlog_info("%s: enter!\n", __func__);
	/* read nfc configure value from nvm */
	ret = nfc_nv_opera(NV_READ, TEL_HUAWEI_NV_NFCCAL_NUMBER, TEL_HUAWEI_NV_NFCCAL_NAME, TEL_HUAWEI_NV_NFCCAL_LEGTH, hex_para);
	if (ret < 0) {
		hwlog_err("%s: get nv error ret: %d!\n", __func__, ret);
		return -1;
	}
	*info = hex_para[FWUPDATE_ON_OFFSET];
	hwlog_info("%s: info = %d.\n", __func__, (int)*info);
	return 0;
}

static ssize_t nfc_fwupdate_on_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char fwupdate_flag = 0;
	(void)st_get_fwupdate_on_info(&fwupdate_flag);
	hwlog_info("%s: read fwupdate_on, returned =%d\n", __func__,  (unsigned char)fwupdate_flag);

	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%d\n", (unsigned char)fwupdate_flag));
}

static ssize_t nfc_fwupdate_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{

	if ('1' == buf[0]) {
		firmware_update = 1;
		pr_info("%s:firmware update success\n", __func__);
	}

	return (ssize_t)count;
}

static ssize_t nfc_fwupdate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, sizeof(firmware_update)+1, "%d", firmware_update));
}

static ssize_t nfc_enable_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct st21nfc_dev *st21nfc_dev = NULL;
	//struct pn547_dev *pn547_dev;
	st21nfc_dev = i2c_get_clientdata(i2c_client_dev);
	if (st21nfc_dev == NULL) {
		pr_err("%s: st21nfc_dev == NULL!\n", __func__);
		return 0;
	}
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d\n", st21nfc_dev->enable_status));
}

static ssize_t nfc_enable_status_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	//struct pn547_dev *pn547_dev;
	struct st21nfc_dev *st21nfc_dev = NULL;
	int val = 0;

	st21nfc_dev = i2c_get_clientdata(i2c_client_dev);
	if (st21nfc_dev == NULL) {
		pr_err("%s: st21nfc_dev == NULL!\n", __func__);
		return 0;
	}
	if (sscanf(buf, "%1d", &val) == 1) {
		if (val == ENABLE_START) {
			st21nfc_dev->enable_status = ENABLE_START;
		} else if (val == ENABLE_END) {
			st21nfc_dev->enable_status = ENABLE_END;
		} else {
			return -EINVAL;
		}
	} else
		return -EINVAL;
	return (ssize_t)count;
}

void st_set_nfc_single_channel(void)
{
	int ret = -1;
	char single_channel_dts_str[MAX_CONFIG_NAME_SIZE] = {0};

	memset(single_channel_dts_str, 0, MAX_CONFIG_NAME_SIZE);
	ret = nfc_get_dts_config_string("st_nfc_single_channel", "st_nfc_single_channel",
		single_channel_dts_str, sizeof(single_channel_dts_str));

	if (ret != 0) {
		memset(single_channel_dts_str, 0, MAX_CONFIG_NAME_SIZE);
		pr_err("%s: can't get nfc single channel dts config\n", __func__);
		g_nfc_single_channel = 0;
		return;
	}
	if (!strcasecmp(single_channel_dts_str, "true")) {
		g_nfc_single_channel = 1;
	}
	pr_err("%s: nfc single channel:%d\n", __func__, g_nfc_single_channel);
	return;
}


void st_set_nfc_card_num(void)
{
	int ret = -1;

	ret = nfc_get_dts_config_u32("st_nfc_card_num", "st_nfc_card_num", &g_nfc_card_num);
	if (ret != 0) {
		g_nfc_card_num = 1;
		pr_err("%s: can't get nfc card num config!\n", __func__);
	}
	return;
}

static ssize_t nfc_card_num_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d\n", (unsigned char)g_nfc_card_num));
}

static ssize_t nfc_chip_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, strlen(g_nfc_chip_type)+1, "%s", g_nfc_chip_type));
}

static int hisi_pmu_reg_operate_by_bit(int reg_addr, int bit_pos, int op)
{
	int ret = 0;
	if (reg_addr <= 0 || bit_pos < 0 || op < 0) {
		pr_err("%s: reg_addr[%x], bit_pos[%d], op[%d], error!\n", __func__, reg_addr, bit_pos, op);
		return -1;
	}
	ret = hisi_pmic_reg_read(reg_addr);
	if (op == CLR_BIT) {
		CLR_PMU_REG_BIT(ret, bit_pos);
	} else {
		SET_PMU_REG_BIT(ret, bit_pos);
	}
	hisi_pmic_reg_write(reg_addr, ret);
	pr_info("%s: reg: 0x%x, pos: %d, value: 0x%x\n", __func__, reg_addr, bit_pos, ret);
	return 0;
}

static void pmu0_svdd_sel_on(void)
{
	pr_err("%s: in g_nfc_svdd_sw: %d\n", __func__, g_nfc_svdd_sw);
	if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V600) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v600.addr, nfc_on_hi6421v600.pos, SET_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V500) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v500.addr, nfc_on_hi6421v500.pos, SET_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V110) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v110.addr, nfc_on_hi6555v110.pos, SET_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V700) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v700.addr, nfc_on_hi6421v700.pos, SET_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V800) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v800.addr, nfc_on_hi6421v800.pos, SET_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V300) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v300.addr, nfc_on_hi6555v300.pos, SET_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V500) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v500.addr, nfc_on_hi6555v500.pos, SET_BIT);
	} else {
		pr_err("%s: pmu gpio don't connect to switch\n", __func__);
	}
}

static void pmu0_svdd_sel_off(void)
{
	pr_err("%s: in g_nfc_svdd_sw: %d\n", __func__, g_nfc_svdd_sw);
	if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V600) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v600.addr, nfc_on_hi6421v600.pos, CLR_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V500) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v500.addr, nfc_on_hi6421v500.pos, CLR_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V110) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v110.addr, nfc_on_hi6555v110.pos, CLR_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V700) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v700.addr, nfc_on_hi6421v700.pos, CLR_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V800) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v800.addr, nfc_on_hi6421v800.pos, CLR_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V300) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v300.addr, nfc_on_hi6555v300.pos, CLR_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V500) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v500.addr, nfc_on_hi6555v500.pos, CLR_BIT);
	} else {
        pr_err("%s: pmu gpio don't connect to switch\n", __func__);
    }
}

static int get_hisi_pmic_reg_status(int reg_addr, int bit_pos)
{
	int ret = 0;
	if (reg_addr <= 0 || bit_pos < 0 || bit_pos > 31) {
		pr_err("%s: reg_addr[%x], bit_pos[%d], error!\n", __func__, reg_addr, bit_pos);
		return -1;
	}
	ret = hisi_pmic_reg_read(reg_addr);

	ret = (ret >> bit_pos) & 0x01;

	pr_err("%s: reg: 0x%x, pos: %d, value: 0x%x\n", __func__, reg_addr, bit_pos, ret);
	return ret;
}

static int get_pmu0_svdd_sel_status(void)
{
    int ret = -1;
	pr_err("%s: in g_nfc_svdd_sw: %d\n", __func__, g_nfc_svdd_sw);
	if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V600) {
		ret = get_hisi_pmic_reg_status(nfc_on_hi6421v600.addr, nfc_on_hi6421v600.pos);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V500) {
		ret = get_hisi_pmic_reg_status(nfc_on_hi6421v500.addr, nfc_on_hi6421v500.pos);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V110) {
		ret = get_hisi_pmic_reg_status(nfc_on_hi6555v110.addr, nfc_on_hi6555v110.pos);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V700) {
		ret = get_hisi_pmic_reg_status(nfc_on_hi6421v700.addr, nfc_on_hi6421v700.pos);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V800) {
		ret = get_hisi_pmic_reg_status(nfc_on_hi6421v800.addr, nfc_on_hi6421v800.pos);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V300) {
		ret = get_hisi_pmic_reg_status(nfc_on_hi6555v300.addr, nfc_on_hi6555v300.pos);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V500) {
		ret = get_hisi_pmic_reg_status(nfc_on_hi6555v500.addr, nfc_on_hi6555v500.pos);
	} else {
        pr_err("%s: pmu gpio don't connect to switch\n", __func__);
    }
    return ret;
}

static ssize_t nfc_svdd_sw_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int val = 0;

	if (sscanf(buf, "%1d", &val) == 1) {
		switch (val) {
		case NFC_SVDD_SW_OFF:
			pmu0_svdd_sel_off();
			break;
		case NFC_SVDD_SW_ON:
			pmu0_svdd_sel_on();
			break;
		default:
			pr_err("%s: svdd switch error, val:%d[0:pulldown, 1:pullup]\n", __func__, val);
			break;
		}
	} else {
		pr_err("%s: val len error\n", __func__);
		return (ssize_t)count;
	}
	pr_err("%s: nfc svdd switch to %d\n", __func__, val);
	return (ssize_t)count;
}

static ssize_t nfc_svdd_sw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int sw_status = 0;

	sw_status = get_pmu0_svdd_sel_status();
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d", sw_status));
}


static ssize_t rd_nfc_sim_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int status = -1;
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	//struct pn547_dev *pn547_dev;
	struct st21nfc_dev *st21nfc_dev;
	st21nfc_dev = i2c_get_clientdata(i2c_client_dev);
	if (st21nfc_dev == NULL) {
		pr_err("%s:	st21nfc_dev == NULL!\n", __func__);
		return status;
	}
	return (ssize_t)(snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE-1, "%d\n", st21nfc_dev->sim_status));
}

void st_set_nfc_chip_type_name(void)
{
	int ret = -1;

	memset(g_nfc_chip_type, 0, MAX_NFC_CHIP_TYPE_SIZE);
	ret = nfc_get_dts_config_string("st_nfc_chip_type", "st_nfc_chip_type",
		g_nfc_chip_type, sizeof(g_nfc_chip_type));

	if (ret != 0) {
		memset(g_nfc_chip_type, 0, MAX_NFC_CHIP_TYPE_SIZE);
		pr_err("%s: can't get nfc nfc_chip_type, default pn547\n", __func__);
		strcpy(g_nfc_chip_type, "st21nfc");
	}

	pr_err("%s: nfc chip type name:%s\n", __func__, g_nfc_chip_type);

	return;
}
static ssize_t nfc_watch_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf(buf, sizeof(g_nfc_watch_type) + 1, "%d", g_nfc_watch_type));
}

static struct device_attribute pn547_attr[] = {

	__ATTR(nfc_fwupdate, 0664, nfc_fwupdate_show, nfc_fwupdate_store),
	__ATTR(nxp_config_name, 0664, nxp_config_name_show, nxp_config_name_store),
	__ATTR(nfc_spec_rf, 0444, nfc_spec_rf_show, NULL),
	__ATTR(st_rf_config_name, 0664, st_rf_config_name_show,
		st_rf_config_name_store),
	__ATTR(nfc_brcm_conf_name, 0664, nfc_brcm_conf_name_show, nfc_brcm_conf_name_store),
	__ATTR(nfc_sim_switch, 0664, nfc_sim_switch_show, nfc_sim_switch_store),
	__ATTR(nfc_sim_status, 0444, nfc_sim_status_show, NULL),
	__ATTR(rd_nfc_sim_status, 0444, rd_nfc_sim_status_show, NULL),
	__ATTR(nfc_enable_status, 0664, nfc_enable_status_show, nfc_enable_status_store),
	__ATTR(nfc_card_num, 0444, nfc_card_num_show, NULL),
	__ATTR(nfc_chip_type, 0444, nfc_chip_type_show, NULL),
	__ATTR(nfc_fw_version, 0444, nfc_fw_version_show, NULL),
	__ATTR(nfcservice_lock, 0664, nfcservice_lock_show, nfcservice_lock_store),
	__ATTR(nfc_svdd_sw, 0664, nfc_svdd_sw_show, nfc_svdd_sw_store),
	__ATTR(nfc_close_type, 0664, nfc_close_type_show, nfc_close_type_store),
	__ATTR(nfc_single_channel, 0444, nfc_single_channel_show, NULL),
	__ATTR(nfc_wired_ese_type, 0444, nfc_wired_ese_info_show, nfc_wired_ese_info_store),
	__ATTR(nfc_activated_se_info, 0664, nfc_activated_se_info_show, nfc_activated_se_info_store),
	__ATTR(nfc_hal_dmd, 0664, nfc_hal_dmd_info_show, nfc_hal_dmd_info_store),
	__ATTR(nfc_calibration, 0444, nfc_calibration_show, NULL),
	__ATTR(nfc_clk_src, 0444, nfc_clk_src_show, NULL),
	__ATTR(nfc_at_result, 0664, nfc_at_result_show, nfc_at_result_store),
	__ATTR(nfc_switch_state, 0664, nfc_switch_state_show, nfc_switch_state_store),
	__ATTR(nfc_ese_num, 0444, nfc_ese_num_show, NULL),
	__ATTR(nfc_recovery_close_nfc, 0664, nfc_recovery_close_nfc_show, nfc_recovery_close_nfc_store),
	__ATTR(nfc_get_rssi, 0664, nfc_get_rssi_show, nfc_get_rssi_store),
	__ATTR(nfc_fwupdate_on, 0444, nfc_fwupdate_on_show, NULL),
	__ATTR(nfc_watch_type, 0444, nfc_watch_type_show, NULL),

};

static int remove_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(pn547_attr); i++) {
		device_remove_file(dev, pn547_attr + i);
	}

	return 0;
}

static int create_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(pn547_attr); i++) {
		if (device_create_file(dev, pn547_attr + i)) {
			goto error;
		}
	}

	return 0;
error:
	for ( ; i >= 0; i--) {
		device_remove_file(dev, pn547_attr + i);
	}

	pr_err("%s:pn547 unable to create sysfs interface.\n", __func__);
	return -1;
}

static int st21nfc_loc_set_polaritymode(struct st21nfc_dev *st21nfc_dev,
					int mode)
{

	struct i2c_client *client = st21nfc_dev->platform_data.client;
	unsigned int irq_type;
	int ret;

	st21nfc_dev->platform_data.polarity_mode = mode;
	/* setup irq_flags */
	switch (mode) {
	case IRQF_TRIGGER_RISING:
		irq_type = IRQ_TYPE_EDGE_RISING;
		st21nfc_dev->platform_data.active_polarity = 1;
		break;
	case IRQF_TRIGGER_FALLING:
		irq_type = IRQ_TYPE_EDGE_FALLING;
		st21nfc_dev->platform_data.active_polarity = 0;
		break;
	case IRQF_TRIGGER_HIGH:
		irq_type = IRQ_TYPE_LEVEL_HIGH;
		st21nfc_dev->platform_data.active_polarity = 1;
		break;
	case IRQF_TRIGGER_LOW:
		irq_type = IRQ_TYPE_LEVEL_LOW;
		st21nfc_dev->platform_data.active_polarity = 0;
		break;
	default:
		irq_type = IRQF_TRIGGER_FALLING;
		st21nfc_dev->platform_data.active_polarity = 0;
		break;
	}
	if (irqIsAttached) {
		free_irq(client->irq, st21nfc_dev);
		irqIsAttached = false;
	}
	/*ret = irq_set_irq_type(client->irq, irq_type);
	if (ret) {
		pr_err("%s : set_irq_type failed\n", __FILE__);
		return -ENODEV;
	}*/
	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	pr_debug("%s : requesting IRQ %d\n", __func__, client->irq);
	st21nfc_dev->irq_enabled = true;

	ret = request_irq(client->irq, st21nfc_dev_irq_handler,
			  mode|IRQF_NO_SUSPEND,
			  client->name, st21nfc_dev);
	if (!ret)
		irqIsAttached = true;

	return ret;
}


static irqreturn_t st21nfc_dev_irq_handler(int irq, void *dev_id)
{
	struct st21nfc_dev *st21nfc_dev = dev_id;
    //hwlog_info("%s : enter  \n", __func__);
	st21nfc_disable_irq(st21nfc_dev);

	/*set a wakelock to avoid entering into suspend */
	if (WAKE_LOCK_TIMEOUT_ENALBE == g_wake_lock_timeout) {
		__pm_wakeup_event(&wlock_read, 5 * HZ);
	} else {
		__pm_wakeup_event(&wlock_read, 1 * HZ);
	}

	/* Wake up waiting readers */
	wake_up(&st21nfc_dev->read_wq);

	return IRQ_HANDLED;
}

static ssize_t st21nfc_dev_read(struct file *filp, char __user *buf,
				size_t count, loff_t *offset)
{
	struct st21nfc_dev *st21nfc_dev = container_of(filp->private_data,
						       struct st21nfc_dev,
						       st21nfc_device);
	char tmp[MAX_BUFFER_SIZE] = { 0};
	int ret;
	int pinlev = 0;
	bool isSuccess = false;
	int retry;

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	//pr_err("%s : reading %zu bytes.\n", __func__, count);
	mutex_lock(&st21nfc_dev->platform_data.read_mutex);
	pinlev = nfc_gpio_get_value(st21nfc_dev->platform_data.irq_gpio);//nfc_gpio_get_value instead of gpio_get_value

	if (((pinlev > 0) && (st21nfc_dev->platform_data.active_polarity == 0))
	    || ((pinlev == 0) && (st21nfc_dev->platform_data.active_polarity == 1))) {
		hwlog_info("%s: read called but no IRQ, return dummy data,pinlev=0x%x\n",__func__,pinlev);
		memset(tmp, 0x7E, count);
		if (copy_to_user(buf, tmp, count)) {
			hwlog_err("%s : failed to copy to user space\n", __func__);
			//pinlev = gpio_get_value(st21nfc_dev->platform_data.irq_gpio);
			//mutex_lock(&st21nfc_dev->platform_data.read_mutex);
			ret = -EFAULT;
			goto fail;
			//return -EFAULT;
		}
        mutex_unlock(&st21nfc_dev->platform_data.read_mutex);
		return count;
	}
	hwlog_info("%s: pinlev=0x%x\n",__func__,pinlev);
	memset(tmp, 0x00, sizeof(tmp));
	/* Read data , we have 3 chances*/
	for (retry = 0; retry < NFC_TRY_NUM; retry++) {
	    ret = i2c_master_recv(st21nfc_dev->platform_data.client, tmp, count);
		hwlog_info("%s : retry = %d, data ret = %d, expect count = %3d, header: %02x%02x%02x%02x%02x\n", __func__,
			retry, ret, (int)count, tmp[0], tmp[1], tmp[2], tmp[3], tmp[4]);
		if (ret == (int)count) {
			isSuccess = true;
			break;
		} else {
			hwlog_info("%s : read data try =%d returned %d\n", __func__, retry, ret);
			msleep(1);
		}
	}

	if (false == isSuccess) {
		hwlog_err("%s : i2c_master_recv returned %d\n", __func__, ret);
		ret = -EIO;
	}
	mutex_unlock(&st21nfc_dev->platform_data.read_mutex);

	if (ret < 0) {
		hwlog_err("%s: i2c_master_recv returned %d\n", __func__, ret);
		return ret;
	}
	if (ret > count) {
		hwlog_err("%s: received too many bytes from i2c (%d)\n",
		       __func__, ret);
		return -EIO;
	}
	if (copy_to_user(buf, tmp, ret)) {
		hwlog_err("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}
	return ret;
fail:
	mutex_unlock(&st21nfc_dev->platform_data.read_mutex);
	return (size_t)ret;
}

static ssize_t st21nfc_dev_write(struct file *filp, const char __user *buf,
				 size_t count, loff_t *offset)
{
	struct st21nfc_dev *st21nfc_dev = NULL;
	char tmp[MAX_BUFFER_SIZE];
	int ret = count;
	int retry;
	bool isSuccess = false;

	st21nfc_dev = container_of(filp->private_data,
				   struct st21nfc_dev, st21nfc_device);
	pr_err("%s: st21nfc_dev ptr %p\n", __func__, st21nfc_dev);

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	memset(tmp, 0x00, sizeof(tmp));
	if (copy_from_user(tmp, buf, count)) {
		hwlog_err("%s : failed to copy from user space\n", __func__);
		return -EFAULT;
	}

	/* Write data */
	for (retry = 0; retry < NFC_TRY_NUM; retry++) {
	    ret = i2c_master_send(st21nfc_dev->platform_data.client, tmp, count);
		hwlog_info("%s : retry = %d, data ret = %d, expect count = %3d, header: %02x%02x%02x%02x%02x\n", __func__,
			retry, ret, (int)count, tmp[0], tmp[1], tmp[2], tmp[3], tmp[4]);
		if (ret == (int)count) {
			isSuccess = true;
			break;
		}else {
			if (retry > 0) {
				hwlog_info("%s : send data retry =%d returned %d\n", __func__, retry, ret);
			}
			msleep(1);
		}
	}
	if (false == isSuccess) {
		hwlog_err("%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}
	return ret;
}

static int st21nfc_dev_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct st21nfc_dev *st21nfc_dev = NULL;
	if (device_open) {
		ret = -EBUSY;
		pr_err("%s : device already opened ret= %d\n", __func__, ret);
		} else {
	device_open = true;
	st21nfc_dev = container_of(filp->private_data,
						       struct st21nfc_dev,
						       st21nfc_device);

	pr_debug("%s : device_open = %d", __func__, device_open);
	pr_debug("%s : %d,%d ", __func__, imajor(inode), iminor(inode));

	pr_debug("%s: st21nfc_dev ptr %p\n", __func__, st21nfc_dev);

	}
	return ret;
}


static int st21nfc_release(struct inode *inode, struct file *file)
{
	device_open = false;
	pr_debug("%s : device_open  = %d\n", __func__, device_open);

	return 0;
}

#ifdef CONFIG_EXT_INPUTHUB
static int st21nfc_sync_sakinfo(unsigned long arg)
{
	int ret;
	int retry;
	struct command command_info;
	nfc_sak_sync_info sak_info;

	if (memset_s(&sak_info, sizeof(nfc_sak_sync_info), 0, sizeof(nfc_sak_sync_info)) != EOK) {
		pr_info("%s memset sak_info err\n", __func__);
		return -1;
	}

	copy_from_user(&sak_info, (nfc_sak_sync_info *)arg, sizeof(nfc_sak_sync_info));
	pr_info("sync sak = 0x%x, cardaid = %s\n", sak_info.sak, sak_info.card_aid);

	command_info.service_id = MC_NFC_WATCH_WALLET_SVR;
	command_info.command_id = NFC_WATCH_SYNC_ORIGINAL_SAK;
	command_info.send_buffer = (unsigned char *)&sak_info;
	command_info.send_buffer_len = sizeof(nfc_sak_sync_info);

	for (retry = 0; retry < NFC_TRY_NUM; retry++) {
		ret = send_command(NFC_CHANNEL, &command_info, 0, NULL);
		if (ret < 0) {
			pr_err("sync sak send_command  ERR, err ret = %d\n", ret);
			msleep(30);
		} else {
			pr_info("sync sak send_command  OK, err ret = %d\n", ret);
			break;
		}
	}
	return ret;
}
#endif

static long st21nfc_dev_ioctl(struct file *filp, unsigned int cmd,
			      unsigned long arg)
{
	struct st21nfc_dev *st21nfc_dev = container_of(filp->private_data,
						       struct st21nfc_dev,
						       st21nfc_device);

	int ret = 0;
#ifdef CONFIG_EXT_INPUTHUB
	struct command command_data;
#endif

	switch (cmd) {

	case ST21NFC_SET_POLARITY_FALLING:
		pr_info(" ### ST21NFC_SET_POLARITY_FALLING ###");
		st21nfc_loc_set_polaritymode(st21nfc_dev, IRQF_TRIGGER_FALLING);
		break;

	case ST21NFC_SET_POLARITY_RISING:
		pr_info(" ### ST21NFC_SET_POLARITY_RISING ###");
		st21nfc_loc_set_polaritymode(st21nfc_dev, IRQF_TRIGGER_RISING);
		break;

	case ST21NFC_SET_POLARITY_LOW:
		pr_info(" ### ST21NFC_SET_POLARITY_LOW ###");
		st21nfc_loc_set_polaritymode(st21nfc_dev, IRQF_TRIGGER_LOW);
		break;

	case ST21NFC_SET_POLARITY_HIGH:
		pr_info(" ### ST21NFC_SET_POLARITY_HIGH ###");
		st21nfc_loc_set_polaritymode(st21nfc_dev, IRQF_TRIGGER_HIGH);
		break;

	case ST21NFC_PULSE_RESET:
#if 0
		pr_info("%s Pulse Request\n", __func__);
		if (st21nfc_dev->platform_data.reset_gpio != 0) {
			/* pulse low for 20 millisecs */
			nfc_gpio_set_value(st21nfc_dev->platform_data.reset_gpio,
				       0);
			msleep(20);
			nfc_gpio_set_value(st21nfc_dev->platform_data.reset_gpio,
				       1);
			pr_info("%s done Pulse Request\n", __func__);
		}
		break;
#else
#ifdef CONFIG_EXT_INPUTHUB
		if (g_nfc_watch_type == NFC_WATCH_GALIEO) {
			int retry;
			int readventry = 0;

			g_nfc_setven_status = 0;
			for (retry = 0; retry < NFC_TRY_NUM; retry++) {
				pr_info("%s : watch_send_command reading bytes.\n", __func__);
				command_data.service_id = MC_NFC_WATCH_WALLET_SVR;
				command_data.command_id = MC_NFC_WATCH_WALLET_SET_VEN_RESET_CMD;
				command_data.send_buffer = NULL;
				command_data.send_buffer_len = 0;
				ret = send_command(NFC_CHANNEL, &command_data, 0, NULL);
				if (ret < 0) {
					pr_err("watch_send_command ERR, err ret = %d\n", ret);
					msleep(30);
				} else {
					pr_info("watch_send_command OK,info ret = %d\n", ret);
					while ((g_nfc_setven_status == 0) && (readventry < MAX_TRY_SET_VEN_MCU)) {
						msleep(20);
						readventry ++;
						hwlog_err("watch_nfc setven status %d,try %d",
							g_nfc_setven_status, readventry);
					}
					break;
				}
			}
			return ret;
		}
#endif
        /* Double pulse is done to exit Quick boot mode.*/
        pr_info("%s Double Pulse Request,reset_gpio=%d\n", __func__,st21nfc_dev->platform_data.reset_gpio);
        if (st21nfc_dev->platform_data.reset_gpio != 0) {

            /* pulse low for 20 millisecs */
            nfc_gpio_set_value(st21nfc_dev->platform_data.reset_gpio,
                    0);
            pr_info("%s,reset_gpio=%d\n", __func__,nfc_gpio_get_value(st21nfc_dev->platform_data.reset_gpio));
            msleep(20);
            nfc_gpio_set_value(st21nfc_dev->platform_data.reset_gpio,
                    1);
            msleep(10);
            /* pulse low for 20 millisecs */
            nfc_gpio_set_value(st21nfc_dev->platform_data.reset_gpio,
                    0);
            msleep(20);
            nfc_gpio_set_value(st21nfc_dev->platform_data.reset_gpio,
                    1);

            pr_info("%s done Double Pulse Request,reset_gpio pin level=%d\n", __func__,nfc_gpio_get_value(st21nfc_dev->platform_data.reset_gpio));
        }
        break;
#endif

	case ST21NFC_GET_WAKEUP:
		/* deliver state of Wake_up_pin as return value of ioctl */
		ret = nfc_gpio_get_value(st21nfc_dev->platform_data.irq_gpio);
		/*
		 * ret shall be equal to 1 if gpio level equals to polarity.
		 * Warning: depending on gpio_get_value implementation,
		 * it can returns a value different than 1 in case of high level
		 */
		if (((ret == 0)
		     && (st21nfc_dev->platform_data.active_polarity == 0))
		    || ((ret > 0)
			&& (st21nfc_dev->platform_data.active_polarity == 1))) {
			ret = 1;
		} else {
			ret = 0;
		}
		hwlog_info("%s get gpio result %d\n", __func__, ret);
		break;
	case ST21NFC_GET_POLARITY:
		ret = st21nfc_dev->platform_data.polarity_mode;
		pr_debug("%s get polarity %d\n", __func__, ret);
		break;
    case ST21NFC_RECOVERY:
        /* For ST21NFCD usage only */
		pr_info("%s Recovery Request\n", __func__);
        if (st21nfc_dev->platform_data.reset_gpio != 0) {
			/* pulse low for 20 millisecs */
			nfc_gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 0);
			msleep(10);
            /* during the reset, force IRQ OUT as DH output instead of input in normal usage */
            ret = gpio_direction_output(st21nfc_dev->platform_data.irq_gpio, 1);
            if (ret) {
                pr_err("%s : gpio_direction_output failed\n", __FILE__);
                ret = -ENODEV;
                break;
            }
            nfc_gpio_set_value(st21nfc_dev->platform_data.irq_gpio, 1);
			msleep(10);
			nfc_gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 1);
			pr_info("%s done Pulse Request\n", __func__);
		}
        msleep(20);
        nfc_gpio_set_value(st21nfc_dev->platform_data.irq_gpio, 0);
        msleep(20);
        nfc_gpio_set_value(st21nfc_dev->platform_data.irq_gpio, 1);
        msleep(20);
        gpio_set_value(st21nfc_dev->platform_data.irq_gpio, 0);
        msleep(20);
        pr_info("%s Recovery procedure finished\n", __func__);
        ret = gpio_direction_input(st21nfc_dev->platform_data.irq_gpio);
        if (ret) {
            pr_err("%s : gpio_direction_input failed\n", __FILE__);
            ret = -ENODEV;
        }
		break;
	case ST21NFC_SET_RESET_LOW:
		// set reset to low when Vps_io is low.
		pr_info("%s set reset to low\n", __func__);
		gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 0);
		break;
#ifdef CONFIG_EXT_INPUTHUB
	case ST21NFC_SYNC_SAK_TO_MCU:
		pr_info(" ### ST21NFC_SYNC_SAK_TO_MCU ###");
		ret = st21nfc_sync_sakinfo(arg);
		break;
#endif
	default:
		pr_err("%s bad ioctl %u\n", __func__, cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static unsigned int st21nfc_poll(struct file *file, poll_table *wait)
{
	struct st21nfc_dev *st21nfc_dev = container_of(file->private_data,
						       struct st21nfc_dev,
						       st21nfc_device);
	unsigned int mask = 0;
	int pinlev = 0;

	/* wait for Wake_up_pin == high  */
	hwlog_info("%s call poll_Wait\n", __func__);
	poll_wait(file, &st21nfc_dev->read_wq, wait);
    //hwlog_info("%s call poll_Wait return\n", __func__);
	pinlev = nfc_gpio_get_value(st21nfc_dev->platform_data.irq_gpio);

	if (((pinlev == 0) && (st21nfc_dev->platform_data.active_polarity == 0))
	    || ((pinlev > 0)
		&& (st21nfc_dev->platform_data.active_polarity == 1))) {
		hwlog_info("%s return ready\n", __func__);
		mask = POLLIN | POLLRDNORM;	/* signal data avail */
		st21nfc_disable_irq(st21nfc_dev);
	} else {
		/* Wake_up_pin  is low. Activate ISR  */
		if (!st21nfc_dev->irq_enabled) {
			hwlog_info("%s enable irq\n", __func__);
			st21nfc_dev->irq_enabled = true;
			enable_irq(st21nfc_dev->platform_data.client->irq);
		} else {
			hwlog_info("%s irq already enabled\n", __func__);
		}

	}
	return mask;
}

static const struct file_operations st21nfc_dev_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = st21nfc_dev_read,
	.write = st21nfc_dev_write,
	.open = st21nfc_dev_open,
	.poll = st21nfc_poll,
	.release = st21nfc_release,

	.unlocked_ioctl = st21nfc_dev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = st21nfc_dev_ioctl
#endif
};

static ssize_t st21nfc_show_i2c_addr(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);

	if (client != NULL)
		return sprintf(buf, "0x%.2x\n", client->addr);
	return 0;
}				/* st21nfc_show_i2c_addr() */

static ssize_t st21nfc_change_i2c_addr(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{

	struct st21nfc_dev *data = dev_get_drvdata(dev);
	long new_addr = 0;

	if (data != NULL && data->platform_data.client != NULL) {
		if (!kstrtol(buf, 10, &new_addr)) {
			mutex_lock(&data->platform_data.read_mutex);
			data->platform_data.client->addr = new_addr;
			mutex_unlock(&data->platform_data.read_mutex);
			return count;
		}
		return -EINVAL;
	}
	return 0;
}				/* st21nfc_change_i2c_addr() */

static ssize_t st21nfc_version(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", DRIVER_VERSION);
}				/* st21nfc_version */

static DEVICE_ATTR(i2c_addr, S_IRUGO | S_IWUSR, st21nfc_show_i2c_addr,
		   st21nfc_change_i2c_addr);

static DEVICE_ATTR(version, S_IRUGO, st21nfc_version, NULL);

static struct attribute *st21nfc_attrs[] = {
	&dev_attr_i2c_addr.attr,
	&dev_attr_version.attr,
	NULL,
};

static struct attribute_group st21nfc_attr_grp = {
	.attrs = st21nfc_attrs,
};

#ifdef CONFIG_OF
static int nfc_parse_dt(struct device *dev, struct st21nfc_platform_data *pdata)
{
	int r = 0;
	struct device_node *np = dev->of_node;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,st21nfc");
	if (IS_ERR_OR_NULL(np)) {
		pr_err("[dsc]%s: cannot find compatible node \"%s\"", __func__, "st,st21nfc");
		return -ENODEV;
	}

	pdata->reset_gpio = of_get_named_gpio(np, "st,reset_gpio", 0);
	if ((!gpio_is_valid(pdata->reset_gpio))) {
		pr_err("[dsc]%s: fail to get reset_gpio\n", __func__);
		return -EINVAL;
	}
	pdata->irq_gpio = of_get_named_gpio(np, "st,irq_gpio", 0);
	if ((!gpio_is_valid(pdata->irq_gpio))) {
		pr_err("[dsc]%s: fail to get irq_gpio\n", __func__);
		return -EINVAL;
	}

	pdata->polarity_mode = 0;



	pr_err("[dsc]%s : get reset_gpio[%d], irq_gpio[%d], polarity_mode[%d]\n",
	       __func__, pdata->reset_gpio, pdata->irq_gpio, pdata->polarity_mode);

	return r;
}
#else
static int nfc_parse_dt(struct device *dev, struct st21nfc_platform_data *pdata)
{
	return 0;
}
#endif


static int st21nfc_get_resource(struct  st21nfc_dev *pdev,	struct i2c_client *client)
{
	int ret = 0;
	char *nfc_clk_status = NULL;
	/*t_pmu_reg_control nfc_clk_hd_reg[] = {{0x000, 0}, {0x0C5, 5}, {0x125, 4}, {0x119, 4}};
	t_pmu_reg_control nfc_clk_en_reg[] = {{0x000, 0}, {0x10A, 2}, {0x110, 0}, {0x000, 0}};
	t_pmu_reg_control nfc_clk_dig_reg[] = {{0x000, 0}, {0x10C, 0}, {0x116, 2}, {0x238, 0}};
	t_pmu_reg_control clk_driver_strength[] = {{0x000, 0}, {0x109, 4}, {0x116, 0}, {0x10D, 0}};
	*/
	t_pmu_reg_control nfc_clk_hd_reg[] = {{0x000, 0}, {0x0C5, 5}, {0x125, 4}, {0x119, 4},
		{0x000, 0}, {0x196, 6}, {0x196, 6}, {0x008C, 4}, {0x0BF, 6}};
	t_pmu_reg_control nfc_clk_en_reg[] = {{0x000, 0}, {0x10A, 2}, {0x110, 0}, {0x000, 0},
		{0x000, 0}, {0x03E, 0}, {0x03E, 0}, {0x0037, 0}, {0x042, 0}};
	t_pmu_reg_control nfc_clk_dig_reg[] = {{0x000, 0}, {0x10C, 0}, {0x116, 2}, {0x238, 0},
		{0x000, 0}, {0x2DC, 0}, {0x2DC, 0}, {0x169, 0}, {0x0A8, 0}};
	t_pmu_reg_control clk_driver_strength[] = {{0x000, 0}, {0x109, 4}, {0x116, 0}, {0x10D, 0},
		{0x000, 0}, {0x188, 0}, {0x188, 0}, {0x07F, 0}, {0x0A7, 0}};
    pr_err("[%s,%d]:joe new get resource\n", __func__, __LINE__);
	ret = of_property_read_string(client->dev.of_node, "clk_status", (const char **)&nfc_clk_status);
	if (ret) {
		pr_err("[%s,%d]:read clk status fail\n", __func__, __LINE__);
		return -ENODEV;
	} else if (!strcmp(nfc_clk_status, "pmu")) {
		pr_err("[%s,%d]:clock source is pmu\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6555")) {
		pr_err("[%s,%d]:clock source is pmu_hi6555\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6555;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6421v600")) {
		pr_err("[%s,%d]:clock source is pmu_hi6421v600\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6421V600;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6555v200")) {
		pr_err("[%s,%d]:clock source is pmu_hi6555v200\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6555V200;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6421v700")) {
		pr_err("[%s,%d]:clock source is pmu_hi6421v700\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6421V700;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6421v800")) {
		pr_err("[%s,%d]:clock source is pmu_hi6421v800\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6421V800;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6421v900")) {
		hwlog_info("[%s,%d]:clock source is pmu_hi6421v900\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6421V900;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6555v300")) {
		pr_err("[%s,%d]:clock source is pmu_hi6555v300\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6555V300;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6555v500")) {
		pr_err("[%s,%d]:clock source is pmu_hi6555v500\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6555V500;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6555v100")) {
		pr_err("[%s,%d]:clock source is pmu_hi6555v100\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6555V100;
	} else if (!strcmp(nfc_clk_status, "xtal")) {
		pr_err("[%s,%d]:clock source is XTAL\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_XTAL;
	} else {
		pr_err("[%s,%d]:clock source is cpu by default\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_CPU;
	}

	pdev->pctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(pdev->pctrl)) {
		pdev->pctrl = NULL;
		pr_err("failed to get clk pin!\n");
		return -ENODEV;
	}

	/* fix udp bug */
	pdev->pins_default = pinctrl_lookup_state(pdev->pctrl, "default");
	pdev->pins_idle = pinctrl_lookup_state(pdev->pctrl, "idle");
	ret = pinctrl_select_state(pdev->pctrl, pdev->pins_default);
	if (ret < 0) {
		pr_err("%s: unapply new state!\n", __func__);
		return -ENODEV;
	}

	if (NFC_CLK_SRC_PMU == g_nfc_clk_src) {
		/*use pmu wifibt clk*/
		pr_err("%s: config pmu clock in register!\n", __func__);
		/*Register 0x0c5 bit5 = 0 -- not disable wifi_clk_en gpio to control wifi_clk output*/
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU].addr,
									nfc_clk_hd_reg[NFC_CLK_SRC_PMU].pos, CLR_BIT);
		/*Register 0x10A bit2 = 0 -- disable internal register to control wifi_clk output*/
		hisi_pmu_reg_operate_by_bit(nfc_clk_en_reg[NFC_CLK_SRC_PMU].addr,
									nfc_clk_en_reg[NFC_CLK_SRC_PMU].pos, CLR_BIT);
		/*Register 0x10C bit0 = 0 -- sine wave(default); 1 --	square wave*/
		hisi_pmu_reg_operate_by_bit(nfc_clk_dig_reg[NFC_CLK_SRC_PMU].addr,
									nfc_clk_dig_reg[NFC_CLK_SRC_PMU].pos, SET_BIT);
		/*Register 0x109 bit5:bit4 = drive-strength
							  00 --3pF//100K;
							  01 --10pF//100K;
							  10 --16pF//100K;
							  11 --25pF//100K */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU].pos + 1, SET_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6555 == g_nfc_clk_src) {
		/*use pmu wifibt clk*/
		pr_err("%s: config pmu_hi6555 clock in register!\n", __func__);
		/*Register 0x125 bit4 = 0 -- not disable wifi_clk_en gpio to control wifi_clk output*/
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6555].addr,
									nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6555].pos, CLR_BIT);
		/*Register 0x110 bit0 = 0 -- disable internal register to control wifi_clk output*/
		hisi_pmu_reg_operate_by_bit(nfc_clk_en_reg[NFC_CLK_SRC_PMU_HI6555].addr,
									nfc_clk_en_reg[NFC_CLK_SRC_PMU_HI6555].pos, CLR_BIT);
		/*Register 0x116 bit2 = sine wave(default):1 --  square wave:0*/
		hisi_pmu_reg_operate_by_bit(nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6555].addr,
									nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6555].pos, CLR_BIT);
		/*Register 0x116 bit1:bit0 = drive-strength
									00 --3pF//100K;
									01 --10pF//100K;
									10 --16pF//100K;
									11 --30pF//100K */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6555].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6555].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6555].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6555].pos + 1, SET_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6421V600 == g_nfc_clk_src) {
		/*use pmu nfc clk*/
		pr_err("%s: config pmu_hi6421v600 clock in register!\n", __func__);
		/*Register 0x119 bit4 = 0 -- not disable nfc_clk_en gpio to control nfc_clk output*/
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V600].addr,
									nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V600].pos, CLR_BIT);
		/*Register 0x238 bit0 = sine wave(default):1 --  square wave:0*/
		hisi_pmu_reg_operate_by_bit(nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6421V600].addr,
									nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6421V600].pos, CLR_BIT);
		/*Register 0x10D bit1:bit0 = drive-strength
									00 --3pF//100K;
									01 --10pF//100K;
									10 --16pF//100K;
									11 --30pF//100K */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V600].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V600].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V600].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V600].pos + 1, CLR_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6421V700 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		pr_err("%s: config pmu_hi6421v700 clock in register!\n", __func__);
		/* Register 0x196 bit6 = 0 -- not disable nfc_clk_en gpio to control nfc_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V700].addr,
			nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V700].pos, CLR_BIT);
		/* Register 0x188 bit1:bit0 = drive-strength
			00 --3pF//100K;
			01 --10pF//100K;
			10 --16pF//100K;
			11 --30pF//100K  */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V700].addr,
			clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V700].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V700].addr,
			clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V700].pos + 1, CLR_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6421V800 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		pr_err("%s: config pmu_hi6421v800 clock in register!\n", __func__);
		/* Register 0x196 bit6 = 0 -- not disable nfc_clk_en gpio to control nfc_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V800].addr,
			nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V800].pos, CLR_BIT);
		/* Register 0x188 bit1:bit0 = drive-strength
			00 --3pF//100K;
			01 --10pF//100K;
			10 --16pF//100K;
			11 --30pF//100K  */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V800].addr,
			clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V800].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V800].addr,
			clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V800].pos + 1, CLR_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6421V900 == g_nfc_clk_src) {
		/*use pmu nfc clk*/
		hwlog_info("%s: config pmu_hi6421v900 clock in register!\n", __func__);
		/*Register 0x196 bit6 = 0 -- not disable nfc_clk_en gpio to control nfc_clk output*/
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V900].addr,
									nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V900].pos, CLR_BIT);
		/*Register 0x2DC bit0 = sine wave(default):1 --  square wave:0*/
		/*hisi_pmu_reg_operate_by_bit(nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6421V900].addr,
									nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6421V900].pos, CLR_BIT);*/
		/*Register 0x188 bit1:bit0 = drive-strength
			00 --3pF//100K;
			01 --10pF//100K;
			10 --16pF//100K;
			11 --30pF//100K */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V900].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V900].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V900].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V900].pos + 1, CLR_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6555V100 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		pr_err("%s: config pmu_hi6555v100 clock in register!\n", __func__);
		/* Register 0x038 bit6 = 0 -- not disable nfc_clk_en gpio to control nfc_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6555V100].addr,
			nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6555V100].pos, CLR_BIT);
		hisi_pmu_reg_operate_by_bit(nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6555V100].addr,
			nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6555V100].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(nfc_clk_en_reg[NFC_CLK_SRC_PMU_HI6555V100].addr,
			nfc_clk_en_reg[NFC_CLK_SRC_PMU_HI6555V100].pos, CLR_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6555V100].addr,
			clk_driver_strength[NFC_CLK_SRC_PMU_HI6555V100].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6555V100].addr,
			clk_driver_strength[NFC_CLK_SRC_PMU_HI6555V100].pos + 1, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6555V100].addr,
			clk_driver_strength[NFC_CLK_SRC_PMU_HI6555V100].pos + 2 , SET_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6555V200 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		/* pmu nfc clk is controlled by pmu, not here */
		pr_err("%s: config pmu_hi6555v200 clock in register!\n", __func__);
	} else if (NFC_CLK_SRC_PMU_HI6555V300 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		/* pmu nfc clk is controlled by pmu, not here */
		pr_err("%s: config pmu_hi6555v300 clock in register!\n", __func__);
	} else if (NFC_CLK_SRC_PMU_HI6555V500 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		/* pmu nfc clk is controlled by pmu, not here */
		pr_err("%s: config pmu_hi6555v500 clock in register!\n", __func__);
	} else if (NFC_CLK_SRC_CPU == g_nfc_clk_src) {
		/* use default soc clk */
		pdev->nfc_clk = devm_clk_get(&client->dev, NFC_CLK_PIN);
		if (IS_ERR(pdev->nfc_clk)) {
			pr_err("failed to get clk out\n");
			return -ENODEV;
		} else {
			ret = clk_set_rate(pdev->nfc_clk, DEFAULT_NFC_CLK_RATE);
			if (ret < 0) {
				return -EINVAL;
			}
		}
	}

	switch (g_nfc_on_type) {
	case NFC_ON_BY_HISI_PMIC:
	case NFC_ON_BY_HI6421V600_PMIC:
	case NFC_ON_BY_HI6555V110_PMIC:
	case NFC_ON_BY_HI6421V700_PMIC:
	case NFC_ON_BY_HI6421V800_PMIC:
	case NFC_ON_BY_HI6555V300_PMIC:
	case NFC_ON_BY_HI6555V500_PMIC:
		pr_err("Nfc on by hisi pmic!\n");
		break;

	default:
		pr_err("Error nfc on type!\n");
		break;
	}

	return 0;
}


static int st21nfc_enable_clk(struct	st21nfc_dev *pdev, int enableflg)
{
	int ret = 0;

	if (g_nfc_clk_src != NFC_CLK_SRC_CPU) {
		pr_err("%s: pmu clk is controlled by clk_req gpio or xtal .\n", __func__);
		return 0;
	}

	if (enableflg == g_clk_status_flag) {
		pr_err("%s: current nfc clk status is the same to enableflag [%d].\n", __func__, enableflg);
		return 0;
	}

	if (enableflg) {
		/*enable clock output*/
		pr_err("%s: enable clock output\n", __func__);
		ret = pinctrl_select_state(pdev->pctrl, pdev->pins_default);
		if (ret < 0) {
			pr_err("%s: unapply new state!\n", __func__);
		}

		ret = clk_prepare_enable(pdev->nfc_clk);
		if (ret < 0) {
			pr_err("%s: clk_enable failed, ret:%d\n", __func__, ret);
		}
	} else {
		/*disable clock output*/
		pr_err("%s: disable clock output\n", __func__);
		clk_disable_unprepare(pdev->nfc_clk);
		ret = pinctrl_select_state(pdev->pctrl, pdev->pins_idle);
		if (ret < 0) {
			pr_err("%s: unapply new state!\n", __func__);
		}
	}
	g_clk_status_flag = enableflg;

	return ret;
}


static void get_nfc_svdd_sw(void)
{
	char nfc_svdd_sw_str[MAX_DETECT_SE_SIZE] = {0};
	int ret = -1;

	memset(nfc_svdd_sw_str, 0, MAX_DETECT_SE_SIZE);
	ret = nfc_get_dts_config_string("nfc_svdd_sw", "nfc_svdd_sw",
		nfc_svdd_sw_str, sizeof(nfc_svdd_sw_str));

	if (ret != 0) {
		memset(nfc_svdd_sw_str, 0, MAX_DETECT_SE_SIZE);
		g_nfc_svdd_sw = NFC_SWP_WITHOUT_SW;
		pr_err("%s: can't get_nfc_svdd_sw\n", __func__);
	} else {
		if (!strncasecmp(nfc_svdd_sw_str, "hi6421v600_pmic", strlen("hi6421v600_pmic"))) {
			g_nfc_svdd_sw = NFC_SWP_SW_HI6421V600;
		} else if (!strncasecmp(nfc_svdd_sw_str, "hi6421v500_pmic", strlen("hi6421v500_pmic"))) {
			g_nfc_svdd_sw = NFC_SWP_SW_HI6421V500;
		} else if (!strncasecmp(nfc_svdd_sw_str, "hi6555v110_pmic", strlen("hi6555v110_pmic"))) {
			g_nfc_svdd_sw = NFC_SWP_SW_HI6555V110;
		} else {
			g_nfc_svdd_sw = NFC_SWP_WITHOUT_SW;
		}
	}
	pr_info("%s: g_nfc_svdd_sw:%d\n", __func__, g_nfc_svdd_sw);
	return;
}

static bool get_spi_cs_gpio()
{
	int ret = nfc_get_dts_config_u32("gpio_spi_cs", "gpio_spi_cs", &g_spi_cs_gpio);
	if (ret != 0) {
		g_spi_cs_gpio = 0;
		pr_err("%s: can't check_ext_gpio\n", __func__);
		return false;
	}
	pr_err("%s: g_spi_cs_gpio: %d\n", __func__, g_spi_cs_gpio);
	return true;
}

static int st21nfc_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int ret;
	int status = -1;
	struct st21nfc_platform_data *platform_data = NULL;
	struct st21nfc_dev *st21nfc_dev = NULL;

	printk("st21nfc_probe\n");
	get_nfc_watch_type();

#ifdef CONFIG_EXT_INPUTHUB
	if (g_nfc_watch_type == NFC_WATCH_GALIEO) {
		ret = register_watch_mcu_nfc_callback();
		if (ret < 0)
			pr_err("%s watch_nfc mcu callback register fail ret=%d\n", __func__, ret);
	}
#endif

	if (client->dev.of_node) {
		platform_data = devm_kzalloc(&client->dev,
			sizeof(struct st21nfc_platform_data), GFP_KERNEL);
		if (!platform_data) {
			dev_err(&client->dev,
			"nfc-nci prob: Failed to allocate memory\n");
			return -ENOMEM;
		}
		pr_info("%s : Parse st21nfc DTS \n", __func__);
		ret = nfc_parse_dt(&client->dev, platform_data);
		if (ret) {
			return ret;
		}
	} else {
		platform_data = client->dev.platform_data;
		pr_info("%s : No st21nfc DTS \n", __func__);
	}
	if (!platform_data) {
		return -EINVAL;
	}
	dev_dbg(&client->dev,"nfc-nci probe: %s, inside nfc-nci flags = %x\n",
	__func__, client->flags);

	if (platform_data == NULL) {
		dev_err(&client->dev, "nfc-nci probe: failed\n");
		return -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s : need I2C_FUNC_I2C\n", __func__);
		return -ENODEV;
	}

	client->adapter->timeout = msecs_to_jiffies(3 * 10);	/* 30ms */
	client->adapter->retries = 0;
	client->addr = 0x8;

	st21nfc_dev = kzalloc(sizeof(*st21nfc_dev), GFP_KERNEL);
	if (st21nfc_dev == NULL) {
		dev_err(&client->dev,
			"failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}

	pr_debug("%s : dev_cb_addr %p\n", __func__, st21nfc_dev);

#ifdef CONFIG_HUAWEI_DSM
	if (!nfc_dclient)
		nfc_dclient = dsm_register_client(&st_dsm_nfc);
#endif

	get_ext_gpio_type();
	get_nfc_svdd_sw();
	ret = st21nfc_get_resource(st21nfc_dev, client);
	if (ret < 0) {
		pr_err("[%s,%d]:st21nfc probe get resource failed\n", __func__, __LINE__);
		ret = -ENODEV;
		goto err_free_buffer;
	}
	get_wake_lock_timeout();
	get_nfc_wired_ese_type();

	/* store for later use */
	st21nfc_dev->platform_data.irq_gpio = platform_data->irq_gpio;
	st21nfc_dev->platform_data.ena_gpio = platform_data->ena_gpio;
	st21nfc_dev->platform_data.reset_gpio = platform_data->reset_gpio;
	st21nfc_dev->platform_data.polarity_mode = platform_data->polarity_mode;
	st21nfc_dev->platform_data.client = client;

	ret = create_sysfs_interfaces(&client->dev);
	if (ret < 0) {
		pr_err("[%s,%d]:Failed to create_sysfs_interfaces\n", __func__, __LINE__);
		ret = -ENODEV;
		goto err_free_buffer;
	}
	ret = sysfs_create_link(NULL, &client->dev.kobj, "nfc");
	if (ret < 0) {
		pr_err("[%s,%d]:Failed to sysfs_create_link\n", __func__, __LINE__);
		ret = -ENODEV;
		goto err_remove_interfaces;
	}

	ret = gpio_request(platform_data->irq_gpio, "irq_gpio");
	if (ret) {
		pr_err("%s : gpio_request failed\n", __FILE__);
		ret = -ENODEV;
		goto err_remove_link;
	}

	ret = gpio_direction_input(platform_data->irq_gpio);
	if (ret) {
		pr_err("%s : gpio_direction_input failed\n", __FILE__);
		ret = -ENODEV;
		goto err_irq_request;
	}

	/* start hw_modify for ST54H card mode power problem */
	if (get_spi_cs_gpio()) {
		ret = gpio_request_one(g_spi_cs_gpio, GPIOF_IN, "gpio_spi_cs");
		if (ret) {
			pr_err("%s : gpio_request spi cs %d failed\n", __func__, g_spi_cs_gpio);
		} else {
			printk("st21nfc set spi cs to HI_Z mode\n");
		}
	}
	/* end hw_modify for ST54H card mode power problem */

	/* initialize irqIsAttached variable */
	irqIsAttached = false;

	/* initialize device_open variable */
	device_open = 0;

	/* handle optional RESET */

	if (platform_data->reset_gpio != 0) {
		ret = gpio_request(platform_data->reset_gpio, "reset_gpio");
		if (ret) {
			pr_err("%s : reset gpio_request failed\n", __FILE__);
			ret = -ENODEV;
			goto err_irq_request;
		}
		ret = gpio_direction_output(platform_data->reset_gpio, 1);
		if (ret) {
			pr_err("%s : reset gpio_direction_output failed\n",
			       __FILE__);
			ret = -ENODEV;
			goto err_reset_request;
		}
	}

	/* set up optional ENA gpio */
	if (platform_data->ena_gpio != 0) {
		ret = gpio_request(platform_data->ena_gpio, "st21nfc_ena");
		if (ret) {
			pr_err("%s : ena gpio_request failed\n", __FILE__);
			ret = -ENODEV;
			goto err_reset_request;
		}
		ret = gpio_direction_output(platform_data->ena_gpio, 0);//xinhua
		if (ret) {
			pr_err("%s : ena gpio_direction_output failed\n",
			       __FILE__);
			ret = -ENODEV;
			goto err_ena_request;
		}
	}
	client->irq = gpio_to_irq(platform_data->irq_gpio);

	enable_irq_wake(client->irq);
	ret = nfc_check_enable(client, st21nfc_dev);
	nfc_gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 0);
	if (ret < 0) {
		ret = -ENODEV;
		pr_err("%s : nfc_check_enable failed\n", __FILE__);
		goto err_ena_request;
	}
	/* init mutex and queues */
	init_waitqueue_head(&st21nfc_dev->read_wq);
	mutex_init(&st21nfc_dev->platform_data.read_mutex);
	spin_lock_init(&st21nfc_dev->irq_enabled_lock);
    pr_err("%s : debug irq_gpio = %d, client-irq =  %d\n", __func__, platform_data->irq_gpio, client->irq);
	st21nfc_dev->st21nfc_device.minor = MISC_DYNAMIC_MINOR;
	st21nfc_dev->st21nfc_device.name = "st21nfc";
	st21nfc_dev->st21nfc_device.fops = &st21nfc_dev_fops;
	st21nfc_dev->st21nfc_device.parent = &client->dev;
    st21nfc_dev->sim_switch = CARD1_SELECT;

	i2c_set_clientdata(client, st21nfc_dev);
	/* Initialize wakelock*/
    //wakeup_source_init(&wlock_read, WAKE_LOCK_SUSPEND, "nfc_read");
    wakeup_source_init(&wlock_read, "nfc_read");
	ret = misc_register(&st21nfc_dev->st21nfc_device);
	if (ret) {
		pr_err("%s : misc_register failed\n", __FILE__);
		goto err_misc_register;
	}

	if (sysfs_create_group(&client->dev.kobj, &st21nfc_attr_grp)) {
		pr_err("%s : sysfs_create_group failed\n", __FILE__);
		goto err_request_irq_failed;
	}
	st21nfc_disable_irq(st21nfc_dev);
	st21nfc_enable_clk(st21nfc_dev, 1);
	status = recovery_close_nfc(client, st21nfc_dev);
	if (status < 0)
		pr_err("%s: recovery_close_nfc error!\n", __func__);
	gpio_set_value(platform_data->ena_gpio, 1);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_NFC);
#endif
    st_set_nfc_chip_type_name();
	st_set_nfc_single_channel();
	st_set_nfc_card_num();
	st_set_nfc_nxp_config_name();
	st_set_nfc_st_rf_config_name();
	st_set_nfc_spec_rf();
	st_set_nfc_ese_num();
	/* set probe flag: st21nfc success */
	set_nfc_dmd_probe_flag(NFC_ST21NFC_PROBE_SUCCESS);

	return 0;

err_request_irq_failed:
	misc_deregister(&st21nfc_dev->st21nfc_device);
err_misc_register:
	mutex_destroy(&st21nfc_dev->platform_data.read_mutex);
    wakeup_source_trash(&wlock_read);
err_ena_request:
	if (platform_data->ena_gpio != 0)
		gpio_free(platform_data->ena_gpio);
err_reset_request:
	if (platform_data->reset_gpio != 0)
		gpio_free(platform_data->reset_gpio);
err_irq_request:
	gpio_free(platform_data->irq_gpio);
err_remove_link:
	sysfs_remove_link(NULL, "nfc");
err_remove_interfaces:
	remove_sysfs_interfaces(&client->dev);
err_free_buffer:
	if (st21nfc_dev->pctrl != NULL) {
		hwlog_err("[%s,%d]: err_free_buffer pinctrl_put \n", __func__, __LINE__);
		devm_pinctrl_put(st21nfc_dev->pctrl);
	}
	kfree(st21nfc_dev);
err_exit:
	/* set probe flag: st21nfc failed */
	set_nfc_dmd_probe_flag(NFC_ST21NFC_PROBE_FAIL);
	setup_timer(&st_nfc_dmd_probe_timer, st_nfc_record_dmd_probe_timerhandler, 0);
	st_nfc_dmd_probe_timer.expires = jiffies + ST_NFC_DMD_PROBE_TIMER;
	add_timer(&st_nfc_dmd_probe_timer);
	return ret;
}

static int st21nfc_remove(struct i2c_client *client)
{
	struct st21nfc_dev *st21nfc_dev = NULL;

	st21nfc_dev = i2c_get_clientdata(client);
	free_irq(client->irq, st21nfc_dev);
	misc_deregister(&st21nfc_dev->st21nfc_device);
    wakeup_source_trash(&wlock_read);
	mutex_destroy(&st21nfc_dev->platform_data.read_mutex);
	gpio_free(st21nfc_dev->platform_data.irq_gpio);
	if (st21nfc_dev->platform_data.ena_gpio != 0)
		gpio_free(st21nfc_dev->platform_data.ena_gpio);
	kfree(st21nfc_dev);
	del_timer(&st_nfc_dmd_probe_timer);
	return 0;
}

static const struct i2c_device_id st21nfc_id[] = {
	{"st21nfc", 0},
	{}
};

static struct of_device_id st21nfc_of_match[] = {
	{ .compatible = "hisilicon,st21nfc", },  //it's same as the compatible of nfc in dts.
	{}
};

// MODULE_DEVICE_TABLE(of, st21nfc_of_match);
static struct i2c_driver st21nfc_driver = {
	.id_table = st21nfc_id,
	.driver = {
		.name	= "st21nfc",
		.owner	= THIS_MODULE,
		.of_match_table	= st21nfc_of_match,
	},
	.probe		= st21nfc_probe,
	.remove		= st21nfc_remove,
};

static struct of_device_id st21nfc_board_of_match[] = {
	{ .compatible = "hisilicon,st21nfc", },
	{}
};

static int st21nfc_platform_probe(struct platform_device *pdev) {
	printk("st21nfc_platform_probe printk\n");
	pr_err("st21nfc_platform_probe pr_err\n");
	return 0;
}

static int st21nfc_platform_remove(struct platform_device *pdev) {
	pr_err("st21nfc_platform_remove\n");
	return 0;
}


static struct platform_driver st21nfc_platform_driver = {
	.probe = st21nfc_platform_probe,
	.remove = st21nfc_platform_remove,
	.driver = {
		.name	= "st21nfc",
		.owner	= THIS_MODULE,
		.of_match_table	= st21nfc_board_of_match,
	},
};

/*
 * module load/unload record keeping
 */

static int __init st21nfc_dev_init(void)
{
	printk("%s: Loading st21nfc driver\n", __func__);
	platform_driver_register(&st21nfc_platform_driver);//add by wuling to fix compilation error
	return i2c_add_driver(&st21nfc_driver);
}

module_init(st21nfc_dev_init);

static void __exit st21nfc_dev_exit(void)
{
	pr_debug("Unloading st21nfc driver\n");
	i2c_del_driver(&st21nfc_driver);
}


/*lint -restore*/
/*
 * g_nfc_ext_gpio bit 0: 1 -- dload use extented gpio, 0 -- dload use soc gpio
 * g_nfc_ext_gpio bit 1: 1 -- irq use extented gpio, 0 -- irq use soc gpio
 */
 /*lint -save -e* */
static void get_ext_gpio_type(void)
{
	int ret = -1;

	ret = nfc_get_dts_config_u32("nfc_ext_gpio", "nfc_ext_gpio", &g_nfc_ext_gpio);

	if (ret != 0) {
		g_nfc_ext_gpio = 0;
		pr_err("%s: can't check_ext_gpio\n", __func__);
	}
	pr_err("%s: g_nfc_ext_gpio:%d\n", __func__, g_nfc_ext_gpio);
	return;
}

static void nfc_gpio_set_value(unsigned int gpio, int val)
{
	if (g_nfc_ext_gpio & DLOAD_EXTENTED_GPIO_MASK) {
		hwlog_info("%s: gpio_set_value_cansleep\n", __func__);
		gpio_set_value_cansleep(gpio, val);
	} else {
		hwlog_info("%s: gpio_set_value\n", __func__);
		gpio_set_value(gpio, val);
	}
}
/*lint -restore*/
static int nfc_gpio_get_value(unsigned int gpio)
{
	int ret = -1;

	if (g_nfc_ext_gpio & IRQ_EXTENTED_GPIO_MASK) {
		/* hwlog_info("%s: gpio_get_value_cansleep\n", __func__); */
		ret = gpio_get_value_cansleep(gpio);
	} else {
		/* hwlog_info("%s: gpio_get_value\n", __func__); */
		ret = gpio_get_value(gpio);
	}
	return ret;
}
/*lint -save -e* */
/*lint -save -e* */
EXPORT_SYMBOL(nfc_gpio_get_value);
static void get_wake_lock_timeout(void)
{
	char wake_lock_str[MAX_WAKE_LOCK_TIMEOUT_SIZE] = {0};
	int ret = -1;

	memset(wake_lock_str, 0, MAX_WAKE_LOCK_TIMEOUT_SIZE);
	ret = nfc_get_dts_config_string("wake_lock_timeout", "wake_lock_timeout",
	wake_lock_str, sizeof(wake_lock_str));

	if (ret != 0) {
		memset(wake_lock_str, 0, MAX_WAKE_LOCK_TIMEOUT_SIZE);
		g_wake_lock_timeout = WAKE_LOCK_TIMEOUT_DISABLE;
		hwlog_err("%s: can't find wake_lock_timeout\n", __func__);
		return;
	} else {
		if (!strncasecmp(wake_lock_str, "ok", strlen("ok"))) {
			g_wake_lock_timeout = WAKE_LOCK_TIMEOUT_ENALBE;
		} else {
			g_wake_lock_timeout = WAKE_LOCK_TIMEOUT_DISABLE;
		}
	}
	hwlog_info("%s: g_wake_lock_timeout:%d\n", __func__, g_wake_lock_timeout);
	return;
}

static int nfc_get_dts_config_string(const char *node_name,
				const char *prop_name,
				char *out_string,
				int out_string_len)
{
	struct device_node *np = NULL;
	const char *out_value = NULL;
	int ret = -1;

	for_each_node_with_property(np, node_name) {
		ret = of_property_read_string(np, prop_name, (const char **)&out_value);
		if (ret != 0) {
			pr_info("%s: can not get prop values with prop_name: %s\n",
							__func__, prop_name);
		} else {
			if (NULL == out_value) {
				pr_info("%s: error out_value = NULL\n", __func__);
				ret = -1;
			} else if (strlen(out_value) >= out_string_len) {
			pr_info("%s: error out_value len :%d >= out_string_len:%d\n",
					__func__, (int)strlen(out_value), (int)out_string_len);
				ret = -1;
			} else {
				strncpy(out_string, out_value, strlen(out_value));
				pr_info("%s: =%s\n", __func__, out_string);
			}
		}
	}
	return ret;
}

static void get_nfc_wired_ese_type(void)
{
	char nfc_on_str[MAX_DETECT_SE_SIZE] = {0};
	int ret = -1;

	memset(nfc_on_str, 0, MAX_DETECT_SE_SIZE);
	ret = nfc_get_dts_config_string("st_nfc_ese_type", "st_nfc_ese_type",
	nfc_on_str, sizeof(nfc_on_str));

	if (ret != 0) {
		memset(nfc_on_str, 0, MAX_DETECT_SE_SIZE);
		g_nfc_ese_type = NFC_WITHOUT_ESE;
		pr_err("%s: can't find nfc_ese_type node\n", __func__);
		return;
	} else {
		if (!strncasecmp(nfc_on_str, "hisee", strlen("hisee"))) {
			g_nfc_ese_type = NFC_ESE_HISEE;
		} else if (!strncasecmp(nfc_on_str, "p61", strlen("p61"))) {
			g_nfc_ese_type = NFC_ESE_P61;
		} else {
			g_nfc_ese_type = NFC_WITHOUT_ESE;
		}
	}
	pr_err("%s: g_nfc_ese_type:%d\n", __func__, g_nfc_ese_type);
	return;
}

static void get_nfc_watch_type(void)
{
	int ret = -1;

	ret = nfc_get_dts_config_u32("st_nfc_watch_type", "st_nfc_watch_type", &g_nfc_watch_type);
	if (ret != 0) {
		g_nfc_watch_type = 0;
		pr_err("%s: can't check_watch_type\n", __func__);
	}
	pr_err("%s: g_nfc_watch_type:%d\n", __func__, g_nfc_watch_type);
	return;
}

#ifdef CONFIG_EXT_INPUTHUB
static int recv_watch_nfc_mcu_cb(unsigned char service_id,
	unsigned char command_id, unsigned char *data, int data_len)
{
	int ret;
	pr_info("%s: recv watch_nfc mcu callback: \n");

	if (service_id != MC_NFC_WATCH_WALLET_SVR) {
		pr_err("%s service_id:%d\n", __func__, service_id);
		return -EINVAL;
	}

	switch (command_id) {
	case MC_NFC_WATCH_WALLET_SET_VEN_RESET_CMD:
		pr_info("watch_nfc mcu ven reset\n");
		g_nfc_setven_status = 1;
		break;
	default:
		break;
	}
	return 0;
}

/*
 * Function:watch_register_mcu_report_callback
 * Description:regtister call back to check ven status
 * Parameters:NA
 * return:return valuse >=0 register successful else register fail for other
 *         reason
 */
static int register_watch_mcu_nfc_callback(void)
{
	pr_info("%s: register watch_nfc mcu callback: \n");
	return register_data_callback(NFC_CHANNEL, &g_nfc_read_mcu_cmds,
		recv_watch_nfc_mcu_cb);
}
#endif
module_exit(st21nfc_dev_exit);

MODULE_AUTHOR("Norbert Kawulski");
MODULE_DESCRIPTION("NFC ST21NFC driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
