#include "drv_ipc.h"
#include "npu_ipc_msg.h"
#include "npu_atf_subsys.h"
#include "npu_platform.h"
#include "npu_adapter.h"
#include "npu_platform_register.h"
#include "bbox/npu_black_box.h"

enum devdrv_powerdown_val
{
	DEVDRV_PD_TS_DONE_FLAG = 0x7B7B,
	DEVDRV_PD_SEC_DONE_FLAG = 0x5D5D,
};

/*tscpu power down: sec reg*/
static int tscpu_pd_sec_reg_polling(u32 expect_val, u32 offset)
{
	u32 __iomem *tscpu_stat0_reg = NULL;
	u32 reg_val = 0;
	u32 times_count = 100000;

	tscpu_stat0_reg = devdrv_plat_get_reg_vaddr(DEVDRV_REG_TS_SUBSYSCTL,
			offset);
	if (tscpu_stat0_reg == NULL)
	{
		devdrv_drv_err("get sc_testreg8 failed.\n");
		return -1;
	}

	reg_val = readl_relaxed(tscpu_stat0_reg);
	devdrv_drv_info("tscpu sc_testreg8 origin val=%x\n", reg_val);

	while(expect_val != reg_val) {
		reg_val = readl_relaxed(tscpu_stat0_reg);
		times_count--;
		udelay(10);
		if(times_count == 0){
			devdrv_drv_err("wait tscpu pd failed. sc_testreg8 = 0x%x\n", reg_val);
			return -1;
		}
	}

	return 0;
}

/*tscpu sec reg power down: sec reg*/
static void set_ts_pd_done_flag(u32 write_val, u32 offset)
{
	u32 __iomem *tscpu_stat0_reg = NULL;

	tscpu_stat0_reg = devdrv_plat_get_reg_vaddr(DEVDRV_REG_TS_SUBSYSCTL,
			offset);
	if (tscpu_stat0_reg == NULL)
	{
		devdrv_drv_err("get sc_testreg8 failed.\n");
		return;
	}

	writel_relaxed(write_val, tscpu_stat0_reg);
	devdrv_drv_debug("tscpu sc_testreg8 addr = %p, write val=0x%x\n",
			tscpu_stat0_reg, write_val);
	return;
}

static int tscpu_idle_polling(u32 expect_val, u32 offset)
{
	u32 __iomem *tscpu_stat0_reg = NULL;
	u32 reg_val = 0;
	u32 times_count = 100000;

	tscpu_stat0_reg = devdrv_plat_get_reg_vaddr(DEVDRV_REG_TS_SUBSYSCTL,
			offset);
	if (tscpu_stat0_reg == NULL)
	{
		devdrv_drv_err("get cfg_stat0 failed.\n");
		return -1;
	}

	reg_val = readl_relaxed(tscpu_stat0_reg);
	devdrv_drv_info("tscpu cfg_stat0 origin val=%x\n", reg_val);

	while(expect_val != (reg_val & expect_val)) {
		reg_val = readl_relaxed(tscpu_stat0_reg);
		times_count--;
		udelay(10);
		if(times_count == 0){
			devdrv_drv_err("wait tscpu idle failed. cfg_stat0 = 0x%x\n",reg_val);
			return -1;
		}
	}

	devdrv_drv_info("tscpu cfg_stat0 val=%x\n", reg_val);
	return 0;
}

static int send_ipc_msg_to_ts(u32 cmd_type, u32 sync_type)
{
	rproc_msg_t ack_buffer[IPCDRV_RPROC_MSG_LENGTH];
	rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH];
	struct ipcdrv_msg_payload *payload = NULL;
	struct ipcdrv_message *ipc_msg = NULL;
	int ret;

	ipc_msg = (struct ipcdrv_message *)msg;
	ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
	ipc_msg->ipc_msg_header.cmd_type = cmd_type;
	ipc_msg->ipc_msg_header.sync_type = sync_type;
	ipc_msg->ipc_msg_header.reserved = 0;
	ipc_msg->ipc_msg_header.msg_length = 1;
	ipc_msg->ipc_msg_header.msg_index = 0;
	payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
	payload->result = 0;

	ipc_msg = (struct ipcdrv_message *)ack_buffer;
	payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
	payload->result = 0;

	ret = hisi_rproc_xfer_async(HISI_RPROC_NPU_MBX2,
					msg,
					IPCDRV_RPROC_MSG_LENGTH);
	if (ret != 0) {
		devdrv_drv_err("hisi_rproc_xfer_async failed.\n");
		return -1;
	}
	return 0;
}

static int inform_ts_power_down(void)
{
	return send_ipc_msg_to_ts(IPCDRV_TS_SUSPEND, IPCDRV_MSG_ASYNC);

	//return devdrv_manager_suspend();
}

int devdrv_plat_powerup_till_npucpu(void)
{
	int ret = 0;
	NPU_DRV_BOOT_TIME_TAG("start npuatf_enable_secmod \n");
	ret = npuatf_enable_secmod(NPU_SEC);
	if (ret != 0)
	{
		devdrv_drv_err("npu subsys power up failed ,ret = 0x%x\n",ret);
		return ret;
	}
	NPU_DRV_BOOT_TIME_TAG("start npuatf_change_slv_secmod\n");
	ret = npuatf_change_slv_secmod(NPU_NONSEC);
	if (ret != 0)
	{
		devdrv_drv_err("change slv secmod fail,ret = 0x%x\n",ret);
		return ret;
	}
	NPU_DRV_BOOT_TIME_TAG("start acpu_gic0_online_ready\n");
	ret = acpu_gic0_online_ready(NPU_SEC);
	if (ret != 0)
	{
		devdrv_drv_err("gic connect fail,ret = 0x%x\n",ret);
		return ret;
	}
	NPU_DRV_BOOT_TIME_TAG("start atf_query_gic0_state\n");
	ret = atf_query_gic0_state(12);
	if (ret != 0)
	{
		devdrv_drv_err("gic connect check fail,ret = 0x%x\n",ret);
		return ret;
	}
	return ret;
}

int devdrv_plat_powerup_till_ts(u32 fw_load_addr, u32 offset)
{
	int ret = 0;
	int poll_times;
	u32 reg_val_ts = 0;
	u32 reg_val_aicpu = 0;
	u32 __iomem *sc_testreg_ts = NULL;
	u32 __iomem *sc_testreg_aicpu = NULL;
	u32 observe_reg_val_old = 0;
	u32 observe_reg_val = 0;
	u32 __iomem *ts_boot_status_observe_reg = NULL;

	NPU_DRV_BOOT_TIME_TAG("start npuatf_start_secmod\n");
	ret = npuatf_start_secmod(fw_load_addr);
	if (ret != 0)
	{
		devdrv_drv_err("ts unreset fail,ret = 0x%x\n",ret);
		return ret;
	}
	NPU_DRV_BOOT_TIME_TAG("end npuatf_start_secmod\n");

	sc_testreg_ts = devdrv_plat_get_reg_vaddr(DEVDRV_REG_TS_SUBSYSCTL,
			offset);
	if (sc_testreg_ts == NULL)
	{
		devdrv_drv_err("get sc_testreg failed.\n");
		return -1;
	}

	ts_boot_status_observe_reg = devdrv_plat_get_reg_vaddr(DEVDRV_REG_TS_SUBSYSCTL,
		TS_BOOT_STATUS_OBSERVATION_REG);
	if (ts_boot_status_observe_reg == NULL)
	{
		devdrv_drv_err("get ts_boot_status_observe_reg failed.\n");
		return -1;
	}

	sc_testreg_aicpu = devdrv_plat_get_reg_vaddr(DEVDRV_REG_TS_SUBSYSCTL,
			DEVDRV_SC_TESTREG1_OFFSET);
	if (sc_testreg_aicpu == NULL)
	{
		devdrv_drv_err("get sc_testreg_aicpu failed.\n");
		return ret;
	}

	mdelay(1);
	NPU_DRV_BOOT_TIME_TAG("start pool ts boot done\n");

	/* poll for TS and AICPU load done */
	poll_times = 0;
	while (1) {
		observe_reg_val = readl_relaxed(ts_boot_status_observe_reg);
		if (observe_reg_val_old != observe_reg_val)
		{
			observe_reg_val_old = observe_reg_val;
			NPU_DRV_BOOT_TIME_TAG("ts_boot_status_observe_reg:0x%x.\n", observe_reg_val);
		}

		reg_val_aicpu = readl_relaxed(sc_testreg_aicpu);

		reg_val_ts = readl_relaxed(sc_testreg_ts);
		if ((reg_val_ts >= DEVDRV_SC_TESTREG_TS_READY) && (reg_val_ts != 0xFFFFFFFFUL)) {
			devdrv_drv_info("TS boot succ, times = %d, reg_val_ts = 0x%x, reg_val_aicpu = 0x%x.\n",
			poll_times, reg_val_ts, reg_val_aicpu);
			break;
		}
		udelay(10);
		poll_times++;
		if (poll_times > 10000) {
			if (reg_val_ts < DEVDRV_SC_TESTREG_TS_READY)
			{
				devdrv_drv_err("TS boot timeout. ts_boot_reg_value 0x%x, observe_reg_val=0x%x\n",
					reg_val_ts, observe_reg_val);
				rdr_system_error(RDR_EXC_TYPE_TS_INIT_EXCEPTION, 0, 0);
			}
			return -1;
		}
	}
	NPU_DRV_BOOT_TIME_TAG("end pool ts boot done\n");

	isb();
	return ret;
}

int devdrv_plat_powerdown_till_npucpu(u32 offset)
{
	int ret = 0;

	//step1. inform ts begining power down
	ret = inform_ts_power_down();
	if (ret) {
		devdrv_drv_err("npuatf_inform_power_down failed ret = %d!\n",ret);
		return ret;
	}

	//step2. wait ts flag, and start set the security register of TS PD flow
	devdrv_drv_warn("TSCPU_GIC_WAKER_PWRR_OPS polling start!\n");
	ret = tscpu_pd_sec_reg_polling(DEVDRV_PD_TS_DONE_FLAG, offset);
	if (ret != 0) {
		devdrv_drv_err("TSCPU_GIC_WAKER_PWRR_OPS_READY"
						" polling failed ret = %d!\n",ret);
		return ret;
	}
	devdrv_drv_warn("TSCPU_GIC_WAKER_PWRR_OPS_READY polling success!\n");

	//update secure register of GIC_WAKER and GIC_PWRR through bl31
	//to end communication between tscpu and npu gic and close GICR0 and
	//GICR1
	ret = npuatf_power_down_ts_secreg();
	if (ret != 0) {
		devdrv_drv_err("end communication between tscpu and npu gic and "
		"close GICR0 and GICR1 failed ret = %d \n", ret);
	}
	devdrv_drv_info("end communication between tscpu and npu gic and "
										"close GICR0 and GICR1 success\n");

	//step3. inform ts that secrity register had been powered down
	set_ts_pd_done_flag(DEVDRV_PD_SEC_DONE_FLAG, offset);

	return 0;
}

int devdrv_plat_powerdown_till_down(u32 expect_val, u32 offset)
{
	int ret = 0;

	//step4 wait tscpu to be idle state
	ret = tscpu_idle_polling(expect_val, offset);
	if (ret != 0){
		devdrv_drv_err("tscpu_idle_polling failed ret = %d !\n",ret);
	}
	devdrv_drv_info("tscpu_idle_polling ret=%d.\n", ret);

	//step5 power down npucpu, npubus and npusubsys through bl31
	ret = npuatf_power_down();
	if (ret != 0) {
		devdrv_drv_err("power down npucpu npu bus and npu "
							"subsystem through bl31 failed ret = %d\n",ret);
	}
	return ret;
}
