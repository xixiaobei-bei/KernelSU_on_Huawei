/*
 * npu_adapter_pm.c
 *
 * about npu adapter pm
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/hisi/npu_pm.h>
#include <linux/random.h>
#include <securec.h>

#include "npu_ipc.h"
#include "npu_ipc_msg.h"
#include "npu_atf_subsys.h"
#include "npu_platform.h"
#include "npu_adapter.h"
#include "npu_platform_register.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_manager_common.h"

static int send_ipc_msg_to_ts_without_payload(u32 cmd_type, u32 sync_type)
{
	rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH] = {0};
	rproc_msg_t ack_buff = 0;
	struct ipcdrv_message *ipc_msg = NULL;
	int ret;

	ipc_msg = (struct ipcdrv_message *)msg;
	ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
	ipc_msg->ipc_msg_header.cmd_type = cmd_type;
	ipc_msg->ipc_msg_header.sync_type = sync_type;
	ipc_msg->ipc_msg_header.reserved = 0;
	ipc_msg->ipc_msg_header.msg_length = 0;
	ipc_msg->ipc_msg_header.msg_index = 0;
	npu_drv_info("cmd_type = %u sync_type = %u\n", cmd_type, sync_type);
	ret = hisi_rproc_xfer_sync(HISI_NPU_ACPU_NPU_MBX3, msg,
		IPCDRV_RPROC_MSG_LENGTH, &ack_buff, 1);
	if (ret != 0) {
		npu_drv_err("hisi_rproc_xfer_async failed, ack_buff:0x%x\n", ack_buff);
		return -1;
	}
	return 0;
}

static int send_ipc_msg_to_ts(u32 cmd_type, u32 sync_type,
	const u8 *send_msg, u32 send_len)
{
	rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH] = {0};
	rproc_msg_t ack_buff = 0;
	struct ipcdrv_message *ipc_msg = NULL;
	int ret;
	u32 msg_cnt = 0;
	u32 left_msg_size = 0;
	u32 msg_length = 0;
	u32 index = 0;

	cond_return_error(send_len > IPCDRV_MSG_MAX_LENGTH, -1,
		"send_len = %u is error\n", send_len);

	if (send_msg == NULL || send_len == 0)
		return send_ipc_msg_to_ts_without_payload(cmd_type, sync_type);

	ipc_msg = (struct ipcdrv_message *)msg;
	ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
	ipc_msg->ipc_msg_header.cmd_type = cmd_type;
	ipc_msg->ipc_msg_header.sync_type = sync_type;
	ipc_msg->ipc_msg_header.reserved = 0;
	ipc_msg->ipc_msg_header.msg_length = send_len;

	msg_cnt = ((send_len - 1) / IPCDRV_MSG_LENGTH) + 1;
	left_msg_size = send_len;
	npu_drv_info("send_len = %u msg_cnt = %u\n", send_len, msg_cnt);
	for (index = 0; index < msg_cnt; index++) {
		msg_length = (left_msg_size > IPCDRV_MSG_LENGTH) ?
			IPCDRV_MSG_LENGTH : left_msg_size;
		ipc_msg->ipc_msg_header.msg_index = index;
		ret = memcpy_s(ipc_msg->ipcdrv_payload, msg_length,
				send_msg + index * IPCDRV_MSG_LENGTH, msg_length);
		cond_return_error(ret != 0, -1,"memcpy failed, index:%u, msg_length:0x%x\n",
			index, msg_length);

		npu_drv_debug(
			"msg_cnt:%u, left_msg_size:0x%x index = %u, msg_index = %u\n",
			msg_cnt, left_msg_size, index, ipc_msg->ipc_msg_header.msg_index);
		ret = hisi_rproc_xfer_sync(HISI_NPU_ACPU_NPU_MBX3, msg,
			IPCDRV_RPROC_MSG_LENGTH, &ack_buff, 1);
		if (ret != 0) {
			npu_drv_err("hisi_rproc_xfer_async failed, ack_buff:0x%x\n",
				ack_buff);
			return -1;
		}

		if (left_msg_size >= IPCDRV_MSG_LENGTH)
			left_msg_size -= IPCDRV_MSG_LENGTH;
	}
	return 0;
}

int npu_powerup_aicore(u64 is_secure, u32 aic_flag)
{
	int ret;

	npu_drv_boot_time_tag("start npuatf_powerup_aicore\n");
	ret = npuatf_powerup_aicore(is_secure, aic_flag);
	if (ret != 0)
		npu_drv_err("npu aicore power up failed ,ret = 0x%x\n", ret);

	return ret;
}

int npu_powerdown_aicore(u64 is_secure, u32 aic_flag)
{
	int ret;

	npu_drv_boot_time_tag("start npuatf_power_down_aicore\n");
	ret = npuatf_power_down_aicore(is_secure, aic_flag);
	if (ret != 0)
		npu_drv_err("npu aicore power down failed ,ret = 0x%x\n", ret);

	return ret;
}

static int inform_ts_power_down(void)
{
	u8 send_msg = 0;

	return send_ipc_msg_to_ts(IPCDRV_TS_SUSPEND, IPCDRV_MSG_ASYNC,
		&send_msg, 0);
}

int npu_plat_send_ts_ctrl_core(uint32_t core_num)
{
	u8 send_msg = (u8)core_num;

	return send_ipc_msg_to_ts(IPCDRV_TS_INFORM_TS_LIMIT_AICORE,
		IPCDRV_MSG_ASYNC, &send_msg, 1);
}

int npu_sync_ts_time(void)
{
	int ret = 0;
	struct npu_time_sync_message time_info;

	time_info.wall_time = current_kernel_time64();
	getrawmonotonic64(&time_info.system_time);
	time_info.ccpu_cycle = npu_read_cntpct();
	npu_drv_info("tv_sec:0x%lx, ccpu_cycle:0x%lx\n",
		time_info.system_time.tv_sec, time_info.ccpu_cycle);
	ret = send_ipc_msg_to_ts(IPCDRV_TS_TIME_SYNC, IPCDRV_MSG_ASYNC,
		(u8 *)&time_info, sizeof(struct npu_time_sync_message));
	return ret;
}

int npu_plat_powerup_till_npucpu(u64 is_secure)
{
	int tmp_ret;
	int ret;

	npu_drv_boot_time_tag("start npuatf_enable_secmod\n");
	ret = npuatf_enable_secmod(is_secure);
	if (ret != 0) {
		npu_drv_err("npu subsys power up failed ,ret = 0x%x\n", ret);
		return ret;
	}

	npu_drv_boot_time_tag("start acpu_gic0_online_ready\n");
	ret = acpu_gic0_online_ready(is_secure);
	if (ret != 0) {
		npu_drv_err("gic connect fail,ret = 0x%x\n", ret);
		return ret;
	}
	npu_drv_boot_time_tag("start atf_query_gic0_state\n");
	tmp_ret = atf_query_gic0_state(NPU_GIC_1);
	// 1 means online, 0 offline
	if (tmp_ret != 1) {
		npu_drv_err("gic connect check fail,tmp_ret = 0x%x\n", tmp_ret);
		ret = -1;
		return ret;
	}

	return ret;
}

extern struct npu_platform_info *s_platform_info;
int npu_plat_powerup_till_ts(u32 is_secure, u32 offset)
{
	int ret;
	u64 canary = 0;

	npu_drv_boot_time_tag("start npuatf_start_secmod\n");
	get_random_bytes(&canary, sizeof(canary));
	if (canary == 0)
		get_random_bytes(&canary, sizeof(canary));

	// 1.unreset ts  2.polling boot status on atf
	ret = npuatf_start_secmod(is_secure, canary);
	if (ret != 0) {
		npu_drv_err("ts unreset fail,ret = 0x%x\n", ret);
		if (npu_rdr_exception_is_count_exceeding(
			RDR_EXC_TYPE_TS_INIT_EXCEPTION) == 0)
			rdr_system_error(RDR_EXC_TYPE_TS_INIT_EXCEPTION, 0, 0);
		return ret;
	}
	npu_drv_boot_time_tag("end npuatf_start_secmod\n");
	return ret;
}

int npu_plat_powerdown_ts(u32 offset, u32 is_secure)
{
	int ret;

	npu_drv_debug("secure is %u\n", is_secure);
	// step1. inform ts begining power down
	if (is_secure != NPU_SEC) {
		ret = inform_ts_power_down();
		if (ret) {
			npu_drv_err("npuatf_inform_power_down failed ret = %d!\n", ret);
			return ret;
		}
	}
	// step2. wait ts flag, and start set the security register of TS PD
	// flow on atf now update secure register of GIC_WAKER and GIC_PWRR
	// through bl31 to end communication between tscpu and npu gic
	// and close GICR0 and GICR1 doorbell do it at atf
	ret = npuatf_power_down_ts_secreg(is_secure);
	if (ret != 0)
		npu_drv_err("end communication between tscpu and npu gic and "
			"close GICR0 and GICR1 failed ret = %d\n", ret);

	npu_drv_info("end communication between tscpu and npu gic and close GICR0 and GICR1 success\n");
	// step3. inform ts that secrity register had been powered down on atf now
	return 0;
}

int npu_plat_powerdown_npucpu(u32 expect_val, u32 mode)
{
	int ret;
	// step4 wait tscpu to be idle state(do it at atf now)
	// step5 power down npucpu, npubus and npusubsys through bl31
	ret = npuatf_power_down(mode);
	if (ret != 0)
		npu_drv_err("power down npucpu npu bus and npu subsystem through bl31 failed ret = %d\n",
			ret);

	return ret;
}

int npu_plat_powerdown_nputop(void)
{
	int ret;

	ret = npu_pm_power_off();
	if (ret != 0)
		npu_drv_err("npu_smc or power_off fail, ret: %d\n", ret);
	return ret;
}

int npu_plat_powerup_tbu(void)
{
	int ret;

	npu_drv_info("start npuatf_enable_tbu\n");
	ret = npuatf_enable_tbu(NPU_NONSEC);
	if (ret != 0) {
		npu_drv_err("npu subsys power up failed ,ret = 0x%x\n", ret);
		return ret;
	}
	return 0;
}

int npu_plat_powerdown_tbu(u32 aic_flag)
{
	int ret;

	npu_drv_info("start npuatf_disable_tbu\n");
	ret = npuatf_disable_tbu(NPU_NONSEC, aic_flag);
	if (ret != 0) {
		npu_drv_err("npu subsys powern down failed ,ret = 0x%x\n", ret);
		return ret;
	}
	return 0;
}

