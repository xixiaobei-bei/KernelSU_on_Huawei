/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: implement the global interface for Boot Detector
 * Author: qidechun
 * Create: 2019-03-05
 */

/* ----includes ---- */
#include <hwbootfail/core/boot_detector.h>
#include <hwbootfail/core/boot_recovery.h>
#include <hwbootfail/core/adapter.h>

/* ----local macroes ---- */
#define BFI_PART_MIN_SIZE (BFI_PART_HDR_SIZE + BF_INFO_SIZE \
	+ BF_META_LOG_SIZE + sizeof(struct bootfail_log_header) \
	+ sizeof(struct recovery_info))
#define SUCCESS_INT 1
#define DEFAULT_DETAIL_INFO "storage can't be found when boot fail happened"
#define INVALID_RRECORD_OFFSET 0xFFFFFFFFFFFFFFFF

/* ----local macro functions ---- */
/* ---- local prototypes ---- */

/*
 * You're strongly recommended to dynamically allocate memory for the struct.
 */
struct bootfail_log {
	struct bootfail_log_header hdr;
	char log_data[0];
} __packed;

struct boot_detector_run_param {
	struct adapter adp;
	int rw_storage_succ;
	int bootfail_saved_in_mem;
	int proc_last_bootfail_succ;
	struct bootfail_proc_param pparam;
	int is_init_succ;
};

/* ---- local static variables ---- */
static struct boot_detector_run_param run_param;

/* ---- local static function prototypes ---- */
static enum bootfail_errorcode __boot_fail_error(
	struct bootfail_proc_param *pparam);

/* ---- function definitions ---- */
static int is_valid_log_meta_info(struct bootfail_meta_log *pinfo)
{
	unsigned char sha256[BF_SIZE_32];

	run_param.adp.sec_funcs.memset_s(sha256,
		(unsigned long)sizeof(sha256), 0,
		(unsigned long)sizeof(sha256));
	run_param.adp.sha256(sha256, sizeof(sha256),
		((unsigned char *)pinfo + sizeof(pinfo->sha256)),
		BF_META_LOG_SIZE - sizeof(pinfo->sha256));
	return (run_param.adp.sec_funcs.memcmp(sha256,
		pinfo->sha256, sizeof(sha256)) == 0) ? 1 : 0;
}

static void update_log_meta_info(struct bootfail_meta_log *pinfo)
{
	run_param.adp.sha256(pinfo->sha256, sizeof(pinfo->sha256),
		((unsigned char *)pinfo + sizeof(pinfo->sha256)),
		BF_META_LOG_SIZE - sizeof(pinfo->sha256));
	(void)run_param.adp.bfi_part.ops.write(
		run_param.adp.bfi_part.dev_path,
		BF_META_LOG_OFFSET, (char *)pinfo, BF_META_LOG_SIZE);
}

static void calc_log_max_count(struct bootfail_meta_log *pinfo)
{
	pinfo->log_max_count = (run_param.adp.bfi_part.part_size -
		BFI_PART_HDR_SIZE - BF_INFO_SIZE - BF_META_LOG_SIZE -
		sizeof(struct recovery_info)) /
		(sizeof(struct bootfail_log_header) +
		run_param.adp.bl_log_ops.log_size +
		run_param.adp.kernel_log_ops.log_size);
	pinfo->log_max_count = min(pinfo->log_max_count,
		(unsigned int)array_size(pinfo->logs_info));
}

static void calc_rrecord_offset(struct bootfail_meta_log *pinfo)
{
	pinfo->rrecord_offset = (unsigned int)(BFI_PART_HDR_SIZE +
		BF_INFO_SIZE + BF_META_LOG_SIZE +
		pinfo->log_max_count * (sizeof(struct bootfail_log_header) +
		run_param.adp.bl_log_ops.log_size +
		run_param.adp.kernel_log_ops.log_size));
}

static void init_bootfail_part(struct bootfail_meta_log *pinfo)
{
	unsigned int i;

	pinfo->write_idx = 0;
	calc_log_max_count(pinfo);
	for (i = 0; i < pinfo->log_max_count; i++) {
		run_param.adp.sec_funcs.strncpy_s(
			pinfo->logs_info[i].bl_log.name,
			(unsigned long)sizeof(pinfo->logs_info[i].bl_log.name),
			run_param.adp.bl_log_ops.log_name,
			(unsigned long)run_param.adp.sec_funcs.strlen(
			run_param.adp.bl_log_ops.log_name));
		pinfo->logs_info[i].bl_log.start = BF_LOG_OFFSET +
			i * (sizeof(struct bootfail_log_header) +
			run_param.adp.bl_log_ops.log_size +
			run_param.adp.kernel_log_ops.log_size) +
			sizeof(struct bootfail_log_header);
		pinfo->logs_info[i].bl_log.size =
			run_param.adp.bl_log_ops.log_size;
		run_param.adp.sec_funcs.strncpy_s(
			pinfo->logs_info[i].kernel_log.name,
			(unsigned long)sizeof(
			pinfo->logs_info[i].kernel_log.name),
			run_param.adp.kernel_log_ops.log_name,
			(unsigned long)run_param.adp.sec_funcs.strlen(
			run_param.adp.kernel_log_ops.log_name));
		pinfo->logs_info[i].kernel_log.start =
			pinfo->logs_info[i].bl_log.start +
			run_param.adp.bl_log_ops.log_size;
		pinfo->logs_info[i].kernel_log.size =
			run_param.adp.kernel_log_ops.log_size;
	}
	calc_rrecord_offset(pinfo);
	update_log_meta_info(pinfo);
}

static void get_bl_log(char *pbuf, unsigned int size)
{
	if (run_param.adp.bl_log_ops.capture_bl_log != NULL)
		run_param.adp.bl_log_ops.capture_bl_log(pbuf, size);
}

static void get_kernel_log(char *pbuf, unsigned int size)
{
	if (run_param.adp.kernel_log_ops.capture_kernel_log != NULL)
		run_param.adp.kernel_log_ops.capture_kernel_log(pbuf, size);
}

static void save_logs(struct bootfail_proc_param *pparam,
	struct bootfail_meta_log *pinfo,
	struct bootfail_log *plog)
{
	get_bl_log(plog->log_data, run_param.adp.bl_log_ops.log_size);
	if (is_upper_stage(pparam->binfo.stage)) {
		get_kernel_log(plog->log_data +
			run_param.adp.bl_log_ops.log_size,
			run_param.adp.kernel_log_ops.log_size);
	}

	if (pinfo->write_idx < pinfo->log_max_count) {
		/* write logs */
		unsigned long long log_size =
			sizeof(struct bootfail_log_header) +
			run_param.adp.bl_log_ops.log_size +
			run_param.adp.kernel_log_ops.log_size;
		(void)run_param.adp.bfi_part.ops.write(
			run_param.adp.bfi_part.dev_path,
			BF_LOG_OFFSET + pinfo->write_idx * (log_size),
			(char *)plog, log_size);

		/* update meta log */
		pinfo->write_idx = (pinfo->write_idx + 1) %
			pinfo->log_max_count;
		pinfo->log_count++;
		if (pinfo->log_count > pinfo->log_max_count)
			pinfo->log_count = pinfo->log_max_count;
		update_log_meta_info(pinfo);
	}
}

static void set_post_action(struct bootfail_proc_param *pparam)
{
	if (pparam->magic == BF_INJECT_MAGIC_NUM) {
		pparam->binfo.post_action = PA_REBOOT;
		return;
	}

	pparam->binfo.post_action = PA_DO_NOTHING;
}

static void post_proc_kernel_bootfail(
	struct recovery_run_param *rparam,
	struct bootfail_proc_param *pparam,
	int rw_storage_succ)
{
	if (rparam->run_prevention != NULL)
		rparam->run_prevention(rparam);
	if (is_do_nothing_fault(rparam->method))
		return;

	/* write bootfail info to raw part or reserved memory */
	set_post_action(pparam);
	if (rw_storage_succ == 0 ||
		run_param.adp.bfi_part.ops.write(
		run_param.adp.bfi_part.dev_path, BF_INFO_OFFSET,
		(char *)pparam, sizeof(*pparam)) != 0) {
		run_param.adp.pyhs_mem_info.ops.write(
			run_param.adp.pyhs_mem_info.base,
			run_param.adp.pyhs_mem_info.size,
			(char *)&pparam->binfo, sizeof(pparam->binfo));
	}
	run_param.adp.sys_ctl.reboot();
}

static void post_proc_bootloader_bootfail(
	struct recovery_run_param *rparam,
	struct bootfail_proc_param *pparam)
{
	if (rparam->run_prevention != NULL)
		rparam->run_prevention(rparam);

	if (pparam->binfo.post_action == PA_REBOOT &&
		run_param.adp.sys_ctl.reboot != NULL) {
		run_param.adp.sys_ctl.reboot();
	} else if (pparam->binfo.post_action == PA_SHUTDOWN &&
		run_param.adp.sys_ctl.shutdown != NULL) {
		run_param.adp.sys_ctl.shutdown();
	}
}

static void post_proc_bootfail(
	struct recovery_run_param *rparam,
	struct bootfail_proc_param *pparam,
	int rw_storage_succ)
{
	if (run_param.adp.is_bootloader() != 0)
		post_proc_bootloader_bootfail(rparam, pparam);
	else
		post_proc_kernel_bootfail(rparam, pparam,
			rw_storage_succ);
}

static enum bootfail_errorcode alloc_mem_for_bootfail_log(
	struct bootfail_log **plog)
{
	unsigned int log_size;

	log_size = sizeof(**plog) +
		run_param.adp.bl_log_ops.log_size +
		run_param.adp.kernel_log_ops.log_size;
	*plog = run_param.adp.mem_ops.malloc(log_size);
	if (*plog == NULL) {
		run_param.adp.print("func: alloc_mem_for_bootfail_log,"
			"error: %d(BF_ALLOC_MEM_FAIL), malloc size: 0x%lx\n",
			BF_ALLOC_MEM_FAIL, (unsigned long)log_size);
		return BF_ALLOC_MEM_FAIL;
	}
	run_param.adp.sec_funcs.memset_s(*plog,
		(unsigned long)log_size, 0,
		(unsigned long)log_size);

	return BF_OK;
}

static void copy_bootfail_info(struct bootfail_proc_param *pparam,
	struct bootfail_log_header *phdr)
{
	if (pparam == NULL || phdr == NULL)
		return;

	run_param.adp.sec_funcs.memcpy_s((char *)&phdr->binfo,
		(unsigned long)sizeof(phdr->binfo), (char *)&pparam->binfo,
		(unsigned long)sizeof(phdr->binfo));
	run_param.adp.sec_funcs.memcpy_s(phdr->detail_info,
		(unsigned long)sizeof(phdr->detail_info), pparam->detail_info,
		(unsigned long)sizeof(phdr->detail_info));
	run_param.adp.sec_funcs.memcpy_s(phdr->log_path,
		(unsigned long)sizeof(phdr->log_path), pparam->log_path,
		(unsigned long)sizeof(phdr->log_path));
	run_param.adp.sec_funcs.memcpy_s(phdr->part_name,
		(unsigned long)sizeof(phdr->part_name), pparam->part_name,
		(unsigned long)sizeof(phdr->part_name));
	run_param.adp.sec_funcs.memcpy_s(phdr->hardware_detail,
		(unsigned long)sizeof(phdr->hardware_detail),
		pparam->hardware_detail,
		(unsigned long)sizeof(phdr->hardware_detail));
	phdr->space_left = pparam->space_left;
	phdr->inodes_left = pparam->inodes_left;
}

static enum bootfail_errorcode proc_bootfail(
	struct bootfail_proc_param *pparam,
	struct bootfail_meta_log *pinfo,
	int rw_storage_succ)
{
	struct bootfail_log *plog = NULL;
	struct bootfail_log_header *phdr = NULL;
	struct recovery_run_param rparam;
	enum bootfail_errorcode errorcode;

	errorcode = alloc_mem_for_bootfail_log(&plog);
	if (errorcode != BF_OK) {
		run_param.adp.mem_ops.free(plog);
		return errorcode;
	}

	/* format log header */
	phdr = (struct bootfail_log_header *)(&plog->hdr);
	run_param.adp.sec_funcs.memset_s((void *)phdr,
		(unsigned long)sizeof(*phdr), 0, (unsigned long)sizeof(*phdr));
	copy_bootfail_info(pparam, phdr);
	run_param.adp.sec_funcs.memset_s((char *)&rparam,
		(unsigned long)sizeof(rparam), 0, (unsigned long)sizeof(rparam));
	rparam.rrecord_offset = rw_storage_succ ?
		(unsigned long long)pinfo->rrecord_offset : (INVALID_RRECORD_OFFSET);
	rparam.pparam = pparam;
	rparam.padp = &run_param.adp;
	(void)do_recovery(&rparam);
	phdr->binfo.recovery_method = rparam.method;
	pparam->binfo.is_perceptible = (pparam->binfo.suggest_recovery_method !=
		METHOD_DO_NOTHING) ? 1 : 0;
	if (rw_storage_succ != 0) {
		if (is_do_nothing_fault(rparam.method)) {
			phdr->binfo.recovery_result = SUCCESS_INT;
			save_logs(pparam, pinfo, plog);
			run_param.adp.mem_ops.free(plog);
			return BF_OK;
		}
	}

	if (pparam->binfo.bootfail_errno == HARDWARE_STORAGE_FAULT &&
		run_param.adp.notify_storage_fault != NULL &&
		rparam.bopd_run_mode != 0) {
		run_param.adp.notify_storage_fault(rparam.bopd_run_mode);
		run_param.adp.mem_ops.free(plog);
		return BF_OK;
	}

	/* capture and save logs and then do post process */
	if (run_param.adp.is_bootloader() != 0)
		save_logs(pparam, pinfo, plog);
	post_proc_bootfail(&rparam, pparam, rw_storage_succ);
	run_param.adp.mem_ops.free(plog);

	return BF_OK;
}

static int is_valid_magic_number(unsigned int magic_num)
{
	return (magic_num == BF_INJECT_MAGIC_NUM ||
		magic_num == BF_SW_MAGIC_NUM ||
		magic_num == BF_HW_MAGIC_NUM) ? 1 : 0;
}

static int is_valid_bootfail_injected(
	struct bootfail_proc_param *pparam)
{
	if (pparam->magic == BF_INJECT_MAGIC_NUM)
		return (pparam->bootfail_count > 0) ? 1 : 0;

	return 1;
}

static int is_valid_suggest_recovery_meth(struct bootfail_proc_param *pparam)
{
	if (pparam->binfo.suggest_recovery_method == METHOD_DO_NOTHING ||
		pparam->binfo.suggest_recovery_method == METHOD_NO_SUGGESTION)
		return 1;

	return 0;
}

static int is_valid_bootfail(struct bootfail_proc_param *pparam)
{
	if (!is_valid_magic_number(pparam->magic)) {
		run_param.adp.print("func: is_valid_bootfail, "
			"Invalid magic number: %x\n",
			pparam->magic);
		return 0;
	}

	if (!is_vaild_bootfail_errno(pparam->binfo.bootfail_errno)) {
		run_param.adp.print("func: is_valid_bootfail, "
			"Invalid bootfail_errno: %x\n",
			pparam->binfo.bootfail_errno);
		return 0;
	}

	if (!is_valid_stage(pparam->binfo.stage)) {
		run_param.adp.print("func: is_valid_bootfail, "
			"Invalid stage: %x\n", pparam->binfo.stage);
		return 0;
	}

	if (!is_valid_suggest_recovery_meth(pparam)) {
		run_param.adp.print("func: is_valid_bootfail, "
			"Invalid suggest_recovery_method: %x\n",
			pparam->binfo.suggest_recovery_method);
		return 0;
	}

	if (!is_valid_bootfail_injected(pparam)) {
		run_param.adp.print(
			"func: is_valid_bootfail, "
			"Invalid injected bootfail info:\n"
			"magic: %x, bootfail_count: %d\n",
			pparam->magic, pparam->bootfail_count);
		return 0;
	}

	return 1;
}

static void update_bootfail_proc_param(
	struct bootfail_proc_param *pparam)
{
	if (pparam->magic == 0)
		run_param.adp.sec_funcs.memset_s(pparam,
			(unsigned long)sizeof(*pparam), 0,
			(unsigned long)sizeof(*pparam));

	if (run_param.bootfail_saved_in_mem != 0) {
		(void)run_param.adp.pyhs_mem_info.ops.write(
			run_param.adp.pyhs_mem_info.base,
			run_param.adp.pyhs_mem_info.size,
			pparam, sizeof(*pparam));
	} else {
		(void)run_param.adp.bfi_part.ops.write(
			run_param.adp.bfi_part.dev_path,
			BF_INFO_OFFSET, (char *)pparam,
			sizeof(*pparam));
	}
}

static enum bootfail_errorcode proc_last_bootfail(
	struct bootfail_proc_param *pparam)
{
	struct bootfail_proc_param *pparam_local = NULL;
	enum bootfail_errorcode errcode;

	pparam_local = run_param.adp.mem_ops.malloc(sizeof(*pparam_local));
	if (pparam_local == NULL) {
		run_param.adp.print("func: proc_last_bootfail, "
			"error: %d(BF_ALLOC_MEM_FAIL), malloc size: 0x%lx\n",
			BF_ALLOC_MEM_FAIL,
			(unsigned long)sizeof(*pparam_local));
		return BF_ALLOC_MEM_FAIL;
	}
	run_param.adp.sec_funcs.memcpy_s(pparam_local,
		(unsigned long)sizeof(*pparam_local),
		pparam, (unsigned long)sizeof(*pparam));
	pparam_local->bootfail_count = (pparam->magic == BF_INJECT_MAGIC_NUM) ?
		(pparam->bootfail_count - 1) : (0);
	pparam_local->magic = (pparam_local->bootfail_count <= 0) ?
		(0) : (pparam->magic);
	update_bootfail_proc_param(pparam_local);

	/* process the upper layer bootfail */
	errcode = __boot_fail_error(pparam);
	run_param.proc_last_bootfail_succ = (errcode == BF_OK) ? (1) : (0);
	run_param.adp.mem_ops.free(pparam_local);

	return errcode;
}

static void read_bootfail_from_storage(void)
{
	if (run_param.adp.bfi_part.ops.read(run_param.adp.bfi_part.dev_path,
		BF_INFO_OFFSET, (char *)(&run_param.pparam),
		sizeof(run_param.pparam)) != 0) {
		run_param.rw_storage_succ = 0;
		run_param.bootfail_saved_in_mem = 1;
	} else {
		if (is_valid_magic_number(run_param.pparam.magic) == 0)
			run_param.bootfail_saved_in_mem = 1;
	}
}

static void read_bootfail_from_pyhs_mem(void)
{
	struct bootfail_proc_param *pparam = NULL;

	pparam = run_param.adp.mem_ops.malloc(sizeof(*pparam));
	if (pparam == NULL) {
		run_param.bootfail_saved_in_mem = 0;
		return;
	}
	run_param.adp.sec_funcs.memset_s(pparam,
		(unsigned long)sizeof(*pparam), 0,
		(unsigned long)sizeof(*pparam));
	run_param.adp.pyhs_mem_info.ops.read((unsigned long)(&pparam->binfo),
		sizeof(pparam->binfo), (void *)run_param.adp.pyhs_mem_info.base,
		run_param.adp.pyhs_mem_info.size);
	pparam->magic = BF_SW_MAGIC_NUM;
	/*lint -e666*/
	run_param.adp.sec_funcs.strncpy_s(pparam->detail_info,
		(unsigned long)sizeof(pparam->detail_info), DEFAULT_DETAIL_INFO,
		(unsigned long)min(run_param.adp.sec_funcs.strlen(
		DEFAULT_DETAIL_INFO), sizeof(pparam->detail_info) - sizeof(char)));
	/*lint +e666*/
	if (is_valid_bootfail(pparam) == 0) {
		run_param.adp.print("func: read_bootfail_from_pyhs_mem, "
			"invalid bootfail info\n");
		run_param.bootfail_saved_in_mem = 0;
		run_param.adp.mem_ops.free(pparam);
		return;
	}
	run_param.adp.sec_funcs.memcpy_s(&run_param.pparam,
		(unsigned long)sizeof(run_param.pparam),
		pparam, (unsigned long)sizeof(*pparam));
	run_param.adp.mem_ops.free(pparam);
}

static void read_last_bootfail(void)
{
	run_param.rw_storage_succ = 1;
	run_param.bootfail_saved_in_mem = 0;
	run_param.adp.sec_funcs.memset_s((char *)(&run_param.pparam),
		(unsigned long)sizeof(run_param.pparam), 0,
		(unsigned long)sizeof(run_param.pparam));
	read_bootfail_from_storage();
	if (run_param.bootfail_saved_in_mem != 0)
		read_bootfail_from_pyhs_mem();
}

static enum bootfail_errorcode proc_upper_layer_bootfail(
	const struct bootfail_proc_param *phelper_pparam)
{
	enum bootfail_errorcode errcode;

	read_last_bootfail();
	if (run_param.rw_storage_succ == 0) {
		run_param.proc_last_bootfail_succ = 0;
		return BF_OK; /* maybe the storage hasn't been inited */
	}
	if (is_valid_bootfail(&run_param.pparam) == 0) {
		run_param.adp.print("func: proc_upper_layer_bootfail, "
			"invalid bootfail info\n");
		run_param.proc_last_bootfail_succ = 1;
		return BF_OK; /* there's no bootfail */
	}
	if (phelper_pparam != NULL) {
		run_param.pparam.binfo.is_updated =
			phelper_pparam->binfo.is_updated;
		run_param.pparam.binfo.is_rooted =
			phelper_pparam->binfo.is_rooted;
	}
	errcode = proc_last_bootfail(&run_param.pparam);

	return errcode;
}

enum bootfail_errorcode get_boot_stage(enum boot_stage *pstage)
{
	enum bootfail_errorcode errorcode;

	if (run_param.is_init_succ == 0) {
		if (run_param.adp.print != NULL)
			run_param.adp.print(
				"func: get_boot_stage, "
				"error: %d(BF_NOT_INIT_SUCC)\n",
				BF_NOT_INIT_SUCC);
		return BF_NOT_INIT_SUCC;
	}

	if (run_param.adp.is_boot_detector_enabled() == 0) {
		run_param.adp.print(
			"func: get_boot_stage, "
			"error: %d(BF_BOOT_DETECTOR_DISABLED)\n",
			BF_BOOT_DETECTOR_DISABLED);
		return BF_BOOT_DETECTOR_DISABLED;
	}

	if (pstage == NULL) {
		run_param.adp.print(
			"func: get_boot_stage, "
			"error: %d(BF_INVALID_PARM)\n",
			BF_INVALID_PARM);
		return BF_INVALID_PARM;
	}

	run_param.adp.mutex_ops.lock();
	errorcode = run_param.adp.stage_ops.get_stage((int *)pstage);
	run_param.adp.mutex_ops.unlock();
	run_param.adp.print("func: get_boot_stage, error: %d, stage: %x\n",
		errorcode, *pstage);

	return errorcode;
}

enum bootfail_errorcode set_boot_stage(enum boot_stage stage)
{
	enum bootfail_errorcode errorcode;
	enum boot_stage curr_stage;

	if (run_param.is_init_succ == 0) {
		if (run_param.adp.print != NULL)
			run_param.adp.print(
				"func: set_boot_stage,"
				"error: %d(BF_NOT_INIT_SUCC)\n",
				BF_NOT_INIT_SUCC);
		return BF_NOT_INIT_SUCC;
	}

	if (run_param.adp.is_boot_detector_enabled() == 0) {
		run_param.adp.print(
			"func: set_boot_stage,"
			"error: %d(BF_BOOT_DETECTOR_DISABLED)\n",
			BF_BOOT_DETECTOR_DISABLED);
		return BF_BOOT_DETECTOR_DISABLED;
	}

	(void)get_boot_stage(&curr_stage);
	if ((curr_stage >= stage) && (curr_stage != BOOT_SUCC_STAGE)) {
		run_param.adp.print("func: set_boot_stage, "
			"curr_stage: %x, stage: %x\n",
			(unsigned int)curr_stage, (unsigned int)stage);
		return BF_OK;
	}

	run_param.adp.mutex_ops.lock();
	errorcode = run_param.adp.stage_ops.set_stage((int)stage);
	run_param.adp.mutex_ops.unlock();
	run_param.adp.print("func: set_boot_stage, error: %d, stage: %x\n",
		errorcode, (unsigned int)stage);

	return errorcode;
}

static void validate_log_meta_info(struct bootfail_proc_param *pparam,
	struct bootfail_meta_log *pinfo)
{
	if ((is_valid_log_meta_info(pinfo) == 0) &&
		pparam->binfo.bootfail_errno != HARDWARE_STORAGE_FAULT) {
		run_param.adp.sec_funcs.memset_s(pinfo,
			(unsigned long)sizeof(*pinfo),
			0, (unsigned long)sizeof(*pinfo));
		init_bootfail_part(pinfo);
	}
}

static enum bootfail_errorcode check_condition_for_boot_fail_err(
	struct bootfail_proc_param *pparam)
{
	if (pparam == NULL) {
		run_param.adp.print(
			"func: check_condition_for_boot_fail_err, "
			"error: %d(BF_INVALID_PARM)\n",
			BF_INVALID_PARM);
		return BF_INVALID_PARM;
	}

	if (is_valid_bootfail(pparam) == 0) {
		run_param.adp.print(
			"func: check_condition_for_boot_fail_err, "
			"error: %d(BF_INVALID_BOOTFAIL_INFO)\n",
			BF_INVALID_BOOTFAIL_INFO);
		return BF_INVALID_BOOTFAIL_INFO;
	}

	return BF_OK;
}

static enum bootfail_errorcode __boot_fail_error(
	struct bootfail_proc_param *pparam)
{
	int rw_storage_succ = 1;
	enum bootfail_errorcode errcode;
	struct bootfail_meta_log *pinfo = NULL;

	run_param.adp.print("func: __boot_fail_error, errno: %x, bootstage: %x, "
		"suggest_recovery_method: %d, exception_type: %x\n",
		pparam->binfo.bootfail_errno, pparam->binfo.stage,
		pparam->binfo.suggest_recovery_method, pparam->binfo.exception_type);

	/* read and validate the meta info */
	pinfo = run_param.adp.mem_ops.malloc(sizeof(*pinfo));
	if (pinfo == NULL) {
		run_param.adp.print("func: __boot_fail_error, "
			"error: %d(BF_ALLOC_MEM_FAIL), malloc size: 0x%lx\n",
			BF_ALLOC_MEM_FAIL, (unsigned long)sizeof(*pinfo));
		return BF_ALLOC_MEM_FAIL;
	}
	run_param.adp.sec_funcs.memset_s((char *)pinfo,
		(unsigned long)sizeof(*pinfo), 0,
		(unsigned long)sizeof(*pinfo));
	run_param.adp.mutex_ops.lock();
	errcode = check_condition_for_boot_fail_err(pparam);
	if (errcode != BF_OK) {
		run_param.adp.mem_ops.free(pinfo);
		run_param.adp.mutex_ops.unlock();
		return errcode;
	}

	if (run_param.adp.bfi_part.ops.read(run_param.adp.bfi_part.dev_path,
		BF_META_LOG_OFFSET, (char *)pinfo, sizeof(*pinfo)) != 0) {
		if (run_param.adp.is_bootloader() != 0) {
			run_param.adp.print("func: __boot_fail_error,"
				"error: %d(BF_READ_PART_FAIL)\n",
				BF_READ_PART_FAIL);
			run_param.adp.mem_ops.free(pinfo);
			run_param.adp.mutex_ops.unlock();
			return BF_READ_PART_FAIL;
		}
		rw_storage_succ = 0;
	} else {
		validate_log_meta_info(pparam, pinfo);
	}

	/* process the boot fail */
	errcode = proc_bootfail(pparam, pinfo, rw_storage_succ);
	run_param.adp.mem_ops.free(pinfo);
	run_param.adp.mutex_ops.unlock();

	return errcode;
}

enum bootfail_errorcode boot_fail_error(
	struct bootfail_proc_param *pparam)
{
	enum bootfail_errorcode errcode;

	if (run_param.is_init_succ == 0) {
		if (run_param.adp.print != NULL)
			run_param.adp.print("func: boot_fail_error, "
				"error: %d(BF_NOT_INIT_SUCC)\n",
				BF_NOT_INIT_SUCC);
		return BF_NOT_INIT_SUCC;
	}
	if (run_param.adp.is_boot_detector_enabled() == 0) {
		run_param.adp.print("func: boot_fail_error, "
			"error: %d(BF_BOOT_DETECTOR_DISABLED)\n",
			BF_BOOT_DETECTOR_DISABLED);
		return BF_BOOT_DETECTOR_DISABLED;
	}
	if (is_valid_magic_number(pparam->magic) == 0)
		pparam->magic = BF_SW_MAGIC_NUM;

	/* process the last boot fail firstly */
	if ((run_param.proc_last_bootfail_succ == 0) &&
		(run_param.adp.is_bootloader() != 0)) {
		(void)proc_upper_layer_bootfail(pparam);
		run_param.adp.print("func: boot_fail_error, "
			"continue process this bootfail\n");
	}

	errcode =  __boot_fail_error(pparam);

	return errcode;
}

static int is_valid_log_name(const struct adapter *padp)
{
	if ((padp->bl_log_ops.log_size > 0) &&
		(padp->sec_funcs.strlen(
		padp->bl_log_ops.log_name) == 0))
		return 0;

	if ((padp->kernel_log_ops.log_size > 0) &&
		(padp->sec_funcs.strlen(
		padp->kernel_log_ops.log_name) == 0))
		return 0;

	return 1;
}

static enum bootfail_errorcode check_param_for_raw_part(
	const struct adapter *padp)
{
	if (padp->bfi_part.dev_path == NULL ||
		padp->bfi_part.part_size < BFI_PART_MIN_SIZE)
		return BF_INVALID_DEV_PATH_OR_SIZE;

	if ((BFI_PART_HDR_SIZE + BF_INFO_SIZE + BF_META_LOG_SIZE +
		sizeof(struct bootfail_log_header) +
		padp->bl_log_ops.log_size +
		padp->kernel_log_ops.log_size +
		sizeof(struct recovery_info)) > padp->bfi_part.part_size)
		return BF_INVALID_DEV_PATH_OR_SIZE;

	return BF_OK;
}

static enum bootfail_errorcode check_raw_part_ops_ptrs(
	const struct adapter *padp)
{
	if (padp->bfi_part.ops.read == NULL ||
		padp->bfi_part.ops.write == NULL)
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_pyhs_mem_ops_ptrs(
	const struct adapter *padp)
{
	if (padp->pyhs_mem_info.ops.read == NULL ||
		padp->pyhs_mem_info.ops.write == NULL)
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_sec_funs_ptrs(
	const struct adapter *padp)
{
	if (padp->sec_funcs.memcpy_s == NULL ||
		padp->sec_funcs.memset_s == NULL ||
		padp->sec_funcs.strncpy_s == NULL ||
		padp->sec_funcs.memcmp == NULL ||
		padp->sec_funcs.strlen == NULL)
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_mem_ops_ptrs(
	const struct adapter *padp)
{
	if (padp->mem_ops.malloc == NULL ||
		padp->mem_ops.free == NULL)
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_sys_ctl_ptrs(
	const struct adapter *padp)
{
	if (padp->sys_ctl.reboot == NULL ||
		padp->sys_ctl.shutdown == NULL)
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_stage_ops_ptrs(
	const struct adapter *padp)
{
	if (padp->stage_ops.set_stage == NULL ||
		padp->stage_ops.get_stage == NULL)
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_verify_ops_ptrs(
	const struct adapter *padp)
{
	if (padp->sha256 == NULL)
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_feature_config_ops_ptrs(
	const struct adapter *padp)
{
	if (padp->is_boot_detector_enabled == NULL ||
		padp->is_boot_recovery_enabled == NULL ||
		padp->is_bootloader == NULL)
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_mutex_ops_ptrs(
	const struct adapter *padp)
{
	if (padp->mutex_ops.lock == NULL ||
		padp->mutex_ops.unlock == NULL)
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_func_ptrs(
	const struct adapter *padp)
{
	if (check_raw_part_ops_ptrs(padp) != BF_OK ||
		check_pyhs_mem_ops_ptrs(padp) != BF_OK ||
		check_sec_funs_ptrs(padp) != BF_OK ||
		check_mem_ops_ptrs(padp) != BF_OK ||
		check_sys_ctl_ptrs(padp) != BF_OK ||
		check_stage_ops_ptrs(padp) != BF_OK ||
		check_verify_ops_ptrs(padp) != BF_OK ||
		check_feature_config_ops_ptrs(padp) != BF_OK ||
		check_mutex_ops_ptrs(padp) != BF_OK ||
		padp->save_misc_msg == NULL ||
		padp->print == NULL)
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_param_for_bootloader(
	const struct adapter *padp)
{
	if ((padp->is_bootloader() != 0) &&
		(padp->notify_storage_fault == NULL ||
		padp->is_bopd_supported == NULL ||
		padp->is_safe_mode_supported == NULL))
		return BF_INVALID_FUNC_PTR;

	return BF_OK;
}

static enum bootfail_errorcode check_param_for_pyhs_mem(
	const struct adapter *padp)
{
	if (padp->pyhs_mem_info.base == 0 ||
		padp->pyhs_mem_info.size < sizeof(struct bootfail_basic_info))
		return BF_INVALID_MEM_ADDR_OR_SIZE;

	return BF_OK;
}

static enum bootfail_errorcode check_param_for_log_name(
	const struct adapter *padp)
{
	if (is_valid_log_name(padp) == 0)
		return BF_INVALID_LOG_NAME;

	return BF_OK;
}

static enum bootfail_errorcode check_params(
	const struct adapter *padp)
{
	enum bootfail_errorcode errorcode;

	errorcode = check_func_ptrs(padp);
	if (errorcode != BF_OK)
		return errorcode;

	errorcode = check_param_for_bootloader(padp);
	if (errorcode != BF_OK)
		return errorcode;

	errorcode = check_param_for_raw_part(padp);
	if (errorcode != BF_OK)
		return errorcode;

	errorcode = check_param_for_pyhs_mem(padp);
	if (errorcode != BF_OK)
		return errorcode;

	errorcode = check_param_for_log_name(padp);
	if (errorcode != BF_OK)
		return errorcode;

	return BF_OK;
}

enum bootfail_errorcode boot_detector_init(struct adapter *padp)
{
	enum bootfail_errorcode errorcode;

	if (padp == NULL)
		return BF_INVALID_PARM;

	if (run_param.is_init_succ != 0)
		return BF_HAS_INITED;

	errorcode = check_params(padp);
	if (errorcode != BF_OK)
		return errorcode;

	padp->mutex_ops.lock();
	padp->sec_funcs.memcpy_s(&run_param.adp,
		(unsigned long)sizeof(run_param.adp),
		padp, (unsigned long)sizeof(*padp));
	run_param.is_init_succ = 1;
	padp->mutex_ops.unlock();
	run_param.adp.print("func: boot_detector_init, "
		"error: %d(BF_OK)\n", BF_OK);

	return BF_OK;
}
