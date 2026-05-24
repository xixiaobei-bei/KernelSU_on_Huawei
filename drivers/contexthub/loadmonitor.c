/* INCLUDE FILES */
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
#include "protocol.h"
#include "loadmonitor.h"
#ifdef CONFIG_INPUTHUB_20
#include "contexthub_route.h"
#else
#include "inputhub_route.h"
#endif
#include "common.h"
#include "libhwsecurec/securec.h"
#include "shmem.h"

#define MODULE_NAME "loadmonitor"

/* STRUCT DEFINITIONS */
typedef struct {
	pkt_header_t             hd;
} loadmonitor_sig_data_req_t;

struct aold_open_param{
	uint32_t                 period;
	uint32_t	             freq;
};

struct LOADMONITOR_RESP_DATA {
	uint32_t                 data_addr;
	uint32_t                 data_len;
};

typedef struct {
	pkt_header_resp_t        comm_resp;
	struct LOADMONITOR_RESP_DATA data;
} loadmonitor_sig_data_resp_t;

/* EXTERN VARIABLES */
extern sys_status_t iom3_sr_status;
extern int g_iom3_state;

/* LOCAL FUNCTIONS */
static bool is_inputhub_ipc_available(void)
{
	if (ST_SLEEP == iom3_sr_status) {
		pr_warn("%s: sensorhub is sleep.\n", __func__);
		return false;
	}

	if (IOM3_ST_RECOVERY == g_iom3_state) {
		pr_warn("%s: sensorhub is recoverying.\n", __func__);
		return false;
	}

	return true;
}

int ao_loadmonitor_enable(unsigned int delay_value, unsigned int freq)
{
	struct aold_open_param *open;
	struct read_info rd;
	int ret;
	pkt_cmn_interval_req_t pkt;
#ifdef CONFIG_INPUTHUB_20
	write_info_t winfo;
#endif

	if (false == is_inputhub_ipc_available()) {
		return -1;
	}

	memset_s(&pkt.param, sizeof(pkt.param), 0, sizeof(pkt.param));
	open = (struct aold_open_param *)&pkt.param;
	open->period = delay_value;
	open->freq = freq;
#ifdef CONFIG_INPUTHUB_20
	winfo.tag = TAG_LOADMONITOR;
	winfo.cmd = CMD_CMN_OPEN_REQ;
	winfo.wr_buf = &pkt.param;
	winfo.wr_len = sizeof(pkt) - sizeof(pkt.hd);
	ret = write_customize_cmd(&winfo, &rd, true);
#else
	pkt.hd.tag       = TAG_LOADMONITOR;
	pkt.hd.cmd       = CMD_CMN_OPEN_REQ;
	pkt.hd.resp      = 1;
	pkt.hd.length    = sizeof(pkt) - sizeof(pkt.hd);
	ret = inputhub_mcu_write_cmd_adapter(&pkt, sizeof(pkt), &rd);
#endif
	if (ret < 0) {
		pr_err("inputhub_mcu_write_cmd_adapter error, ret is %d!\r\n", ret);
		return -1;
	} else {
		if (rd.errno)
			return rd.errno;
	}
	return 0;
}

int ao_loadmonitor_disable(void)
{
	struct read_info rd;
	int ret;
#ifdef CONFIG_INPUTHUB_20
	write_info_t winfo;
#else
	pkt_cmn_close_req_t pkt;
#endif

	if (false == is_inputhub_ipc_available()) {
		return -1;
	}

#ifdef CONFIG_INPUTHUB_20
	winfo.tag = TAG_LOADMONITOR;
	winfo.cmd = CMD_CMN_CLOSE_REQ;
	winfo.wr_buf = NULL;
	winfo.wr_len = 0;
	ret = write_customize_cmd(&winfo, &rd, true);
#else
	pkt.hd.tag       = TAG_LOADMONITOR;
	pkt.hd.cmd       = CMD_CMN_CLOSE_REQ;
	pkt.hd.resp      = 1;
	pkt.hd.length    = sizeof(pkt) - sizeof(pkt.hd);
	memset_s(&pkt.close_param, sizeof(pkt.close_param), 0, sizeof(pkt.close_param));
	ret = inputhub_mcu_write_cmd_adapter(&pkt, sizeof(pkt), &rd);
#endif
	if (ret < 0) {
		pr_err("inputhub_mcu_write_cmd_adapter error, ret is %d!\r\n", ret);
		return -1;
	} else {
		if (rd.errno)
			return rd.errno;
	}
	return 0;
}

int32_t _ao_loadmonitor_read(void *data, uint32_t len)
{
	int ret;
	struct read_info rd;
	struct LOADMONITOR_RESP_DATA *resp_dt;
	static void __iomem *p_data = NULL;
	size_t dt_len;
#ifdef CONFIG_INPUTHUB_20
	write_info_t winfo;
#else
	pkt_header_t pkt;
#endif

	if (data == NULL) {
		pr_err("[%s] data is err.\n", __func__);
		return -1;
	}

	if (false == is_inputhub_ipc_available()) {
		return -1;
	}

#ifdef CONFIG_INPUTHUB_20
	winfo.tag = TAG_LOADMONITOR;
	winfo.cmd = CMD_READ_AO_MONITOR_SENSOR;
	winfo.wr_buf = NULL;
	winfo.wr_len = 0;
	ret = write_customize_cmd(&winfo, &rd, true);
#else
	pkt.tag      = TAG_LOADMONITOR;
	pkt.cmd      = CMD_READ_AO_MONITOR_SENSOR;
	pkt.resp     = 1;
	pkt.length   = 0;
	ret = inputhub_mcu_write_cmd_adapter(&pkt, sizeof(pkt), &rd);
#endif
	if (ret) {
		pr_err("inputhub_mcu_write_cmd_adapter error, ret is %d!\r\n", ret);
		return ret;
	}

	if (rd.errno < 0) {
		pr_err("rd error, ret is %d!\r\n", rd.errno);
		return rd.errno;
	} else {
		resp_dt = (struct LOADMONITOR_RESP_DATA *)rd.data;
		dt_len = (size_t)resp_dt->data_len;
		if (dt_len > (size_t)DDR_LOADMONITOR_PHYMEM_SIZE)
			dt_len = (size_t)DDR_LOADMONITOR_PHYMEM_SIZE;

		if (NULL == p_data) {
			p_data = (char *)ioremap_wc((size_t)DDR_LOADMONITOR_PHYMEM_BASE_AP, DDR_LOADMONITOR_PHYMEM_SIZE);
			if (NULL == p_data) {
				pr_err("remap address error, addr %x, len %d!\r\n", resp_dt->data_addr, resp_dt->data_len);
				return -1;
			}
		}
		memcpy_s(data, len, p_data, dt_len);
	}
	return 0;
}

/*lint -e528 -e753*/
MODULE_ALIAS("platform:contexthub"MODULE_NAME);
MODULE_LICENSE("GPL v2");

