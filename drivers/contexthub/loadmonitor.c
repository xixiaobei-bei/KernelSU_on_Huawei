/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub loadmonitor driver.
 * Author: Huawei
 * Create: 2017-06-13
 */

/* INCLUDE FILES */
#include "loadmonitor.h"
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <iomcu_ddr_map.h>
#include <securec.h>
#include "protocol.h"
#include "contexthub_route.h"
#include "common.h"
#include "shmem.h"

#define MODULE_NAME "loadmonitor"
#ifdef __LLT_UT__
#define STATIC
#else
#define STATIC static
#endif

/* STRUCT DEFINITIONS */
typedef struct {
	struct pkt_header         hd;
} loadmonitor_sig_data_req_t;

struct aold_open_param{
	uint32_t             period;
	uint32_t             freq;
};

struct LOADMONITOR_RESP_DATA {
	uint32_t             data_addr;
	uint32_t             data_len;
};

typedef struct {
	struct pkt_subcmd_resp    comm_resp;
	struct LOADMONITOR_RESP_DATA data;
} loadmonitor_sig_data_resp_t;

struct ao_loadmonitor_info {
	struct completion    complete;
	struct spinlock      lock;
	loadmonitor_sig_data_resp_t resp;
};

sys_status_t __weak iom3_sr_status = ST_WAKEUP;
int __weak g_iom3_state = IOM3_ST_NORMAL;
static int g_ao_loadmonitor_init = 0;
static struct ao_loadmonitor_info g_ao_loadmonitor_info;

/* LOCAL FUNCTIONS */
/*
 * bool函数，判断inputbhub IPC通信正常，返回一个ｂｏｏｌ值
 */
STATIC bool is_inputhub_ipc_available(void)
{
	if (ST_SLEEP == iom3_sr_status) {
		pr_warn("%s: sensorhub is sleep.\n", __func__);
		return false;
	}

	if (IOM3_ST_RECOVERY == g_iom3_state) {
		pr_warn("%s: sensorhub is recoverying.\n", __func__);
		return false;
	}

	if (g_ao_loadmonitor_init == 0) {
		pr_warn("%s: not init yet.\n", __func__);
		return false;
	}

	return true;
}

STATIC int ao_loadmonitor_data_from_mcu(const struct pkt_header *head)
{
	loadmonitor_sig_data_resp_t *p = (loadmonitor_sig_data_resp_t *)head;
	int ret;

	if (p == NULL) {
		pr_err("%s: data NULL\n", __func__);
		return -EINVAL;
	}

	if ((p->comm_resp.hd.tag != TAG_LOADMONITOR) || (p->comm_resp.hd.cmd != CMD_READ_AO_MONITOR_SENSOR_RESP)) {
		pr_warn("%s: invalid tag[0x%x] cmd[0x%x]\n", __func__, p->comm_resp.hd.tag, p->comm_resp.hd.cmd);
		return -EINVAL;
	}

	if (completion_done(&g_ao_loadmonitor_info.complete)) {
		pr_warn("%s: no read waiting\n", __func__);
		return 0;
	}

	spin_lock(&g_ao_loadmonitor_info.lock);
	ret = memcpy_s(&g_ao_loadmonitor_info.resp, sizeof(loadmonitor_sig_data_resp_t), p, sizeof(loadmonitor_sig_data_resp_t));
	spin_unlock(&g_ao_loadmonitor_info.lock);

	if (ret != EOK) {
		pr_err("%s: memcpy fail, ret[%d]\n", __func__, ret);
		return -EFAULT;
	}

	complete(&g_ao_loadmonitor_info.complete);

	return 0;
}
/* lint -e86 */
STATIC int ao_loadmonitor_init(void)
{
	int ret;

	ret = register_mcu_event_notifier(TAG_LOADMONITOR, CMD_READ_AO_MONITOR_SENSOR_RESP, ao_loadmonitor_data_from_mcu);
	if (ret != 0) {
		pr_err("%s: register notifier fail, ret[%d]\n", __func__, ret);
		return ret;
	}

	init_completion(&g_ao_loadmonitor_info.complete);
	spin_lock_init(&g_ao_loadmonitor_info.lock);

	return 0;
}
/* lint +e86 */
/*
 * 发送IPC通知contexthub使能loadmonitor
 */
int ao_loadmonitor_enable(unsigned int delay_value, unsigned int freq)
{
	struct aold_open_param *open = NULL;
	struct read_info rd;
	int ret;
	pkt_cmn_interval_req_t pkt;
	struct write_info winfo;

	if (get_contexthub_dts_status() != 0)
		return -EINVAL;

	if (g_ao_loadmonitor_init == 0) {
		ret = ao_loadmonitor_init();
		if (ret != 0) {
			pr_err("%s: init fail, ret[%d]\n", __func__, ret);
			return ret;
		}
		g_ao_loadmonitor_init = 1;
	}

	if (false == is_inputhub_ipc_available()) {
		return -EINVAL;
	}

	(void)memset_s(&pkt.param, sizeof(pkt.param), 0, sizeof(pkt.param));
	open = (struct aold_open_param *)&pkt.param;
	open->period = delay_value;
	open->freq = freq;
	winfo.tag = TAG_LOADMONITOR;
	winfo.cmd = CMD_CMN_OPEN_REQ;
	winfo.wr_buf = &pkt.param;
	winfo.wr_len = sizeof(pkt) - sizeof(pkt.hd);
	ret = write_customize_cmd(&winfo, &rd, true);
	if (ret < 0) {
		pr_err("inputhub_mcu_write_cmd_adapter error, ret is %d!\r\n", ret);
		return ret;
	} else {
		if (rd.errno)
			return rd.errno;
	}
	return 0;
}
/*
 * 发送IPC通知contexthub关闭loadmonitor
 */
int ao_loadmonitor_disable(void)
{
	struct read_info rd;
	int ret;
	struct write_info winfo;

	if (get_contexthub_dts_status() != 0)
		return -EINVAL;

	if (false == is_inputhub_ipc_available()) {
		return -EINVAL;
	}

	winfo.tag = TAG_LOADMONITOR;
	winfo.cmd = CMD_CMN_CLOSE_REQ;
	winfo.wr_buf = NULL;
	winfo.wr_len = 0;
	ret = write_customize_cmd(&winfo, &rd, true);
	if (ret < 0) {
		pr_err("inputhub_mcu_write_cmd_adapter error, ret is %d!\r\n", ret);
		return ret;
	} else {
		if (rd.errno)
			return rd.errno;
	}
	return 0;
}
/*
 * 发送IPC通知contexthub读取loadmonitor数据
 */
 /* lint -e446 */
int32_t _ao_loadmonitor_read(void *data, uint32_t len)
{
	int ret, i;
	struct LOADMONITOR_SIGS *blank_data = NULL;
	struct LOADMONITOR_RESP_DATA *resp_dt = NULL;
	static void __iomem *p_data;
	size_t dt_len;
	unsigned long left;
	struct write_info winfo;

	if (data == NULL) {
		pr_err("[%s] data is err.\n", __func__);
		return -EINVAL;
	}

	if (get_contexthub_dts_status() != 0) {
		blank_data  = (struct LOADMONITOR_SIGS *)data;
		(void)memset_s(blank_data, sizeof(*blank_data), 0, sizeof(*blank_data));
		for (i = 0; i < MAX_SIG_CNT_PER_IP; i++)
			blank_data->sig[i].samples = 1;
		return -ENODEV;
	}

	if (false == is_inputhub_ipc_available()) {
		return -EINVAL;
	}

	reinit_completion(&g_ao_loadmonitor_info.complete);

	winfo.tag = TAG_LOADMONITOR;
	winfo.cmd = CMD_READ_AO_MONITOR_SENSOR;
	winfo.wr_buf = NULL;
	winfo.wr_len = 0;
	ret = write_customize_cmd(&winfo, NULL, true);
	if (ret) {
		pr_err("%s: error, ret is %d!\r\n", __func__, ret);
		return ret;
	}

	left = wait_for_completion_timeout(&g_ao_loadmonitor_info.complete, msecs_to_jiffies(3000));
	if (left == 0) {
		pr_warn("%s: timeout, left[%lu]\n", __func__, left);
		return -EINVAL;
	} else {
		spin_lock(&g_ao_loadmonitor_info.lock);
		resp_dt = &g_ao_loadmonitor_info.resp.data;
		dt_len = (size_t)resp_dt->data_len;
		spin_unlock(&g_ao_loadmonitor_info.lock);
		if (dt_len > (size_t)DDR_LOADMONITOR_PHYMEM_SIZE)
			dt_len = (size_t)DDR_LOADMONITOR_PHYMEM_SIZE;

		if (p_data == NULL) {
			p_data = (char *)ioremap_wc((size_t)DDR_LOADMONITOR_PHYMEM_BASE_AP, DDR_LOADMONITOR_PHYMEM_SIZE);
			if (p_data == NULL) {
				pr_err("%s: remap address error, len %d!\r\n", __func__, resp_dt->data_len);
				return -ENOMEM;
			}
		}

		ret = memcpy_s(data, len, p_data, dt_len);
		if (EOK != ret) {
			pr_err("%s: memcpy data fail, ret[%d]\n", __func__, ret);
			return -EFAULT;
		}
	}
	return 0;
}
/* lint +e446 */
/*lint -e528 -e753*/
MODULE_ALIAS("platform:contexthub"MODULE_NAME);
MODULE_LICENSE("GPL v2");

