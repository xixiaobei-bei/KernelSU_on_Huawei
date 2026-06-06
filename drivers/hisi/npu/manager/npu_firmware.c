#include <linux/io.h>

#include "drv_log.h"
#include "npu_firmware.h"
#include "npu_common.h"
#include "npu_platform.h"
#include <linux/hisi/rdr_pub.h>
#include "bbox/npu_black_box.h"

#define NPU_FIRMWARE_GET_FILE_SIZE_FAIL  (-1)

u32 devdrv_get_firmware_remap_size(int type);

static struct devdrv_ts_bin_info dev_ts_bin_info = {0};
static struct devdrv_mem_desc* fw_mem_desc[DEVDRV_FW_TYPE_MAX] = {0};

ssize_t devdrv_read_file(struct file *fp, void *firmware_dst_addr,
							loff_t fsize, loff_t pos,int buf_len)
{
	ssize_t ret;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs((mm_segment_t)KERNEL_DS);/*lint !e501 */
	ret = vfs_read(fp, firmware_dst_addr, fsize, &pos);
	set_fs(old_fs);

	return ret;
}

loff_t devdrv_get_file_size(const char *path)
{
	int error;
	loff_t filesize = -1;
	struct kstat statbuff;
	mm_segment_t old_fs;

	old_fs = get_fs();
	/*lint -emacro(501,KERNEL_DS) */
	set_fs((mm_segment_t)KERNEL_DS);/*lint !e501 */

	error = vfs_stat(path, &statbuff);
	if (error < 0) {
		devdrv_drv_err("vfs_stat failed ret = %d\n ", error);
		set_fs(old_fs);
		return filesize;
	} else {
		filesize = statbuff.size;
	}
	set_fs(old_fs);
	return filesize;
}

static void devdrv_get_sha256_check(u8 *buf,
					struct devdrv_check_sum *out)
{
	char str[128];
	int i;

	for (i = 0; i < DEVDRV_TS_BIN_CHEKC_LEN; i++) {
		out->check[i] = *(u8 *)buf;
		buf += sizeof(u8);
		snprintf(&str[i * 2], 3, "%02x", out->check[i]);//lint !e679
	}

	str[i * 2] = '\0';//lint !e679
	devdrv_drv_debug("get sha256 from file: %s\n", str);
}

static void devdrv_get_segment_info(u8 *buf,
						u32 segment_num,int buff_len)
{
	u32 i;

	for (i = 0; i < segment_num; i++) {
		devdrv_drv_info("segment id: %d: .\n", i);
		dev_ts_bin_info.segment[i].offset = *(u32 *)buf;
		buf += sizeof(u32);
		dev_ts_bin_info.segment[i].len = *(u32 *)buf;
		buf += sizeof(u32);
		devdrv_get_sha256_check(
			buf,&dev_ts_bin_info.segment[i].segment_check);
		buf += sizeof(struct devdrv_check_sum);

		devdrv_drv_debug("segment len: %d.\n",
							dev_ts_bin_info.segment[i].len);
	}
}

static int devdrv_judge_segment_invalid(void)
{
	u32 segment_end;
	u32 i;

	for (i = 0; i < dev_ts_bin_info.segment_num; i++) {
		segment_end = dev_ts_bin_info.segment[i].offset +
					  dev_ts_bin_info.segment[i].len;

		if (segment_end > dev_ts_bin_info.fw_data_len) {
			devdrv_drv_err("segment %d is invalid, "
							"offset: %d, len: %d, firmware len: %d.\n",
							i, dev_ts_bin_info.segment[i].offset,
							dev_ts_bin_info.segment[i].len,
							dev_ts_bin_info.fw_data_len);
			return 1;
		}
	}

	return 0;
}

int devdrv_judge_bin_validity(struct file *fp, loff_t *fsize,
								loff_t *pos, u32 *ts_check_file)
{
	struct devdrv_check_sum check;
	u32 segment_num;
	loff_t read_pos;
	ssize_t read_num;
	u32 fw_len_allign;
	u32 fw_len;
	u8 buf[BUFF_LEN] = {0};
	read_pos = 0;

	read_num = devdrv_read_file(fp, buf, sizeof(u32) * 2, read_pos,BUFF_LEN);
	if (read_num < (ssize_t)(sizeof(u32) * 2)) {
		devdrv_drv_err("vfs_read failed.\n");
		return -1;
	}

	if (buf[0] != 'T' || buf[1] != 'S' || buf[2] != 'F' || buf[3] != 'W') {
		devdrv_drv_info("firmware is not a 'TSFW'.\n");
		ts_check_file = 0;
		dev_ts_bin_info.ts_check_file = 0;
		return 0;
	}

	devdrv_drv_debug("firmware is a TS firmware.\n");
	fw_len = *(u32 *)(buf + sizeof(u32));

	if (fw_len == 0 || fw_len >= (*fsize - sizeof(u32) * 2)) {
		devdrv_drv_err("TS firmware length is invalid: %d.\n", fw_len);
		return -1;
	}

	devdrv_drv_debug("TS firmware length: %d.\n", fw_len);

	dev_ts_bin_info.ts_check_file = 1;
	*ts_check_file = 1;
	*fsize = fw_len;
	*pos = 8;

	fw_len_allign = 4 - (fw_len % 4);
	fw_len_allign = (fw_len % 4 > 0) ? (fw_len + fw_len_allign) : fw_len;

	devdrv_drv_debug("fw_len_allign length: %d.\n", fw_len_allign);

	/* get segment info */
	read_pos = 8 + fw_len_allign;

	read_num = devdrv_read_file(fp, buf, sizeof(struct devdrv_check_sum) +
										sizeof(u32) * 2, read_pos,BUFF_LEN);
	if (read_num < (int)(sizeof(struct devdrv_check_sum) + sizeof(u32) * 2)) {
		devdrv_drv_err("vfs_read failed.\n");
		return -1;
	}

	segment_num = *(u32 *)buf;
	if (segment_num > DEVDRV_TS_BIN_MAX_SEGMENT_NUM) {
		devdrv_drv_err("too much segments: %d.\n", segment_num);
		return -1;
	}

	devdrv_drv_debug("segment number: %d.\n", segment_num);

	devdrv_get_sha256_check((buf + 2 * sizeof(u32)), &check);

	read_pos = 8 + fw_len_allign + 2 * sizeof(u32) +
						sizeof(struct devdrv_check_sum);
	read_num = devdrv_read_file(fp, buf,
			(long)(unsigned)sizeof(struct devdrv_ts_bin_segment) *segment_num,
			read_pos,BUFF_LEN);
	if (read_num < (int)(sizeof(struct devdrv_ts_bin_segment) * segment_num)) {
		devdrv_drv_err("vfs_read failed.\n");
		return -1;
	}

	memcpy(&dev_ts_bin_info.fw_data_check, &check,
					sizeof(struct devdrv_check_sum));
	dev_ts_bin_info.fw_data_len = fw_len;
	dev_ts_bin_info.segment_num = segment_num;
	devdrv_get_segment_info(buf, segment_num, BUFF_LEN);

	if (devdrv_judge_segment_invalid()) {
		devdrv_drv_err("devdrv_is_segment_invalid return error.\n");
		return -1;
	}

	return 0;
}

bool devdrv_load_firmware(struct file *fp, loff_t fsize, loff_t pos, u32 type)
{
	int ret = 0;
	char *fw_load_addr = NULL;
	u64 max_size = (TS_FW_REMAP_SIZE < fsize) ? fsize:TS_FW_REMAP_SIZE;

	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	if (plat_info == NULL)
	{
		devdrv_drv_err("devdrv_plat_get_info failed.\r\n");
		return false;
	}

	if (type == DEVDRV_FW_TYPE_AICPU) {
		max_size = fsize;
		fw_load_addr = ioremap_nocache(
			devdrv_get_firmware_phy_addr(DEVDRV_FW_TYPE_AICPU), max_size);
	}
	else {
		if (fsize > TS_FW_MAX_SIZE) {
			devdrv_drv_err("ts firmware size %d is too large.\r\n", (u32)fsize);
			return false;
		}
		fw_load_addr = ioremap_nocache(
			devdrv_get_firmware_phy_addr(DEVDRV_FW_TYPE_TS), max_size);
	}

	if (fw_load_addr == NULL) {
		devdrv_drv_err("ioremap_nocache failed.\n");
		return false;
	}
	devdrv_drv_debug("max_size = 0x%llx ,fw_load_addr = %pK \n ",
						max_size, fw_load_addr);
	devdrv_drv_debug("pos %d type %d \n ", (u32)pos, type);



	ret = DEVDRV_PLAT_GET_RES_FW_PROC(plat_info)(fp, fsize, pos, (u64)(uintptr_t)fw_load_addr);
	if (ret != true)
	{
		devdrv_drv_err("npu_fw_proc failed, fsize is %d\r\n", (u32)fsize);
        iounmap(fw_load_addr);
		return false;
	}

	iounmap(fw_load_addr);
	return true;
}

int devdrv_load_cpu_fw_by_type(const char *firmware_path, u32 type)
{
	int ret = -1;
	loff_t pos = 0, fsize = 0;
	u32 ts_check_flag = 0;
	struct file *fp = NULL;

	if (firmware_path == NULL) {
		devdrv_drv_err("firmware path is null.\n");
		return -1;
	}

	if (type > DEVDRV_FW_TYPE_TS) {
		devdrv_drv_err("firmware type is invalid.\n");
		return -1;
	}

	fsize = devdrv_get_file_size(firmware_path);
	devdrv_drv_debug("firmware size is : %lld\n", fsize);
    if (fsize == NPU_FIRMWARE_GET_FILE_SIZE_FAIL) {
        devdrv_drv_err("get file size failed.\n");
        return -1;
    }

	fp = filp_open(firmware_path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		devdrv_drv_err("can't open firmware file.\n");
		return -1;
	}

	ret = devdrv_judge_bin_validity(fp, &fsize, &pos, &ts_check_flag);
	if (ret != 0) {
		devdrv_drv_err("invalid bin type %d .\n", type);
		filp_close(fp, NULL);
		return ret;
	}

	if (devdrv_load_firmware(fp, fsize, pos, type) == false) {
		devdrv_drv_err("can't load firmware file %d.\n", type);
		filp_close(fp, NULL);
		return ret;
	}

	filp_close(fp, NULL);
	return 0;
}

int devdrv_load_cpu_fw(void)
{
	int ret;
	static u32 s_ts_fw_load_flag = 0;

	ret = devdrv_load_cpu_fw_by_type(DEVDRV_AICPU_BINARY_PATH,
											DEVDRV_FW_TYPE_AICPU);
	if (ret != 0) {
		devdrv_drv_err("load ai cpu fw failed.\n");
		return ret;
	}

	if(s_ts_fw_load_flag == 0)
	{
		NPU_DRV_BOOT_TIME_TAG("start LOAD TS FW \n");
		ret = devdrv_load_cpu_fw_by_type(DEVDRV_TS_BINARY_PATH,
											DEVDRV_FW_TYPE_TS);
		NPU_DRV_BOOT_TIME_TAG("END LOAD TS FW \n");
		if (ret != 0) {
			devdrv_drv_err("load ts cpu fw failed.\n");
			/* bbox : load ts cpu fw failed */
			rdr_system_error((u32)RDR_EXC_TYPE_TS_INIT_EXCEPTION, 0, 0);
			return ret;
		}
		s_ts_fw_load_flag = 1;
	}

	return 0;
}

void devdrv_get_firmware_mem_desc(void)
{
	struct devdrv_platform_info* plat_info = NULL;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_irq failed.\n");
		return;
	}

	fw_mem_desc[DEVDRV_FW_TYPE_AICPU]
			= DEVDRV_PLAT_GET_AIFW_BUF(plat_info);
	fw_mem_desc[DEVDRV_FW_TYPE_TS]
			= DEVDRV_PLAT_GET_TSFW_BUF(plat_info);
	return;
}

u32 devdrv_get_firmware_remap_size(int type)
{
	if (fw_mem_desc[type] == NULL) {
		devdrv_get_firmware_mem_desc();
	}
	return fw_mem_desc[type]->len;
}

u64 devdrv_get_firmware_phy_addr(int type)
{
	if (fw_mem_desc[type] == NULL) {
		devdrv_get_firmware_mem_desc();
	}
	return (u64)fw_mem_desc[type]->base;
}
EXPORT_SYMBOL(devdrv_get_firmware_phy_addr);


