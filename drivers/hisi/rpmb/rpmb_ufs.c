

#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/bootdevice.h>
#include <linux/slab.h>
#include <linux/hisi/rpmb.h>

#include <asm/uaccess.h>
#include <scsi/sg.h>

#include "hisi_rpmb.h"


struct combine_cmd {
	struct ufs_blk_ioc_data *data;
	struct rpmb_frame *frame;
	int is_write;
	unsigned short blocks;
	unsigned short blksz;
	u8 *sdb_command;
	u8 *sense_buffer;
	struct rpmb_frame *transfer_frame;
};

#define CMD_PARA_SET(_cmd, _data, _frame, _is_write, _blocks, _blksz, _sdb_command, _sense_buffer, _transfer_frame) \
	do {                                                                   \
		(_cmd)->data = (_data);                                        \
		(_cmd)->frame = (_frame);                                      \
		(_cmd)->is_write = (_is_write);                                \
		(_cmd)->blocks = (_blocks);                                    \
		(_cmd)->blksz = (_blksz);                                      \
		(_cmd)->sdb_command = (_sdb_command);                          \
		(_cmd)->sense_buffer = (_sense_buffer);                        \
		(_cmd)->transfer_frame = (_transfer_frame);                    \
	} while (0)

extern long blk_scsi_kern_ioctl(unsigned int fd, unsigned int cmd,
				unsigned long arg);

DEFINE_MUTEX(rpmb_ufs_cmd_lock);

int ufs_bsg_ioctl_rpmb_cmd(enum func_id id, struct ufs_blk_ioc_rpmb_data *rdata)
{
	int32_t fd;
	int32_t i;
	int32_t ret = -RPMB_ERR_GENERAL;
	int32_t retry_times = 0;
	mm_segment_t oldfs;

	mutex_lock(&rpmb_ufs_cmd_lock);
	/*lint -e501*/
	oldfs = get_fs();
	set_fs(get_ds());
	/*lint +e501*/
	fd = (int32_t)sys_open(UFS_RPMB_BLOCK_DEVICE_NAME, O_RDWR,
			       (int32_t)S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd < 0) {
		pr_err("rpmb ufs file device open failed, and fd = %d!\n", fd);
		ret = RPMB_ERR_BLKDEV;
		goto out;
	}
	for (i = 0; i < UFS_IOC_MAX_RPMB_CMD; i++) {
		if (!rdata->data[i].siv.request_len)
			break;
		while (retry_times < RPMB_IOCTL_RETRY_TIMES) {
			ret = (int32_t)blk_scsi_kern_ioctl(
				(unsigned int)fd, SG_IO,
				(unsigned long)(&(rdata->data[i].siv)));

			if (!ret && !rdata->data[i].siv.info)
				break;
			else {
				pr_err("rpmb ioctl [%d], ret %d, status code: "
				       "driver_status = %d "
				       "transport_status = %d "
				       "device_status = %d!\n",
				       i, ret, rdata->data[i].siv.driver_status,
				       rdata->data[i].siv.transport_status,
				       rdata->data[i].siv.device_status);
				pr_err("ufs rpmb sense data is %d\n",
				       (*((u8 *)rdata->data[i].siv.response +
					  2)));
			}
			retry_times++;
		}

		if (retry_times == RPMB_IOCTL_RETRY_TIMES) {
			pr_err("rpmb ufs ioctl retry failed, total retry "
			       "times is %d!\n",
			       retry_times);
			ret = RPMB_ERR_IOCTL;
			goto out;
		}
		if (id == RPMB_FUNC_ID_SE) {
			if (rdata->data[i].siv.din_xfer_len)
				memcpy(rdata->data[i].buf, /* unsafe_function_ignore: memcpy */
				       (void *)rdata->data[i].siv.din_xferp,
				       rdata->data[i].buf_bytes); /*[false alarm]: buf size is same when alloc*/
			else
				memcpy(rdata->data[i].buf, /* unsafe_function_ignore: memcpy */
				       (void *)rdata->data[i].siv.dout_xferp,
				       rdata->data[i].buf_bytes); /*[false alarm]: buf size is same when alloc*/
		}
		retry_times = 0;
	}
out:
	if (fd >= 0)
		sys_close((unsigned int)fd);
	set_fs(oldfs);
	mutex_unlock(&rpmb_ufs_cmd_lock);
	return ret;
}

/*
 * To distinguish with UFS2.1 and UFS3.0, we must specify one region number in
 * parameter list, please confirm that region is valid from caller instead of
 * this function.
 */
void ufs_get_cdb_rpmb_command(uint32_t opcode, uint32_t size, uint8_t *cmd,
			      uint8_t region)
{
	enum rpmb_version dev_ver;

	if (rpmb_get_dev_ver(&dev_ver))
		return;

	switch (opcode) {
	case UFS_OP_SECURITY_PROTOCOL_IN:
		cmd[0] = UFS_OP_SECURITY_PROTOCOL_IN;
		cmd[1] = SECURITY_PROTOCOL; /* Manju updated from 0x00 */
		if (dev_ver >= RPMB_VER_UFS_30)
			cmd[2] = region;
		else
			cmd[2] = 0;
		cmd[3] = 0x01;
		cmd[4] = 0x00;
		cmd[5] = 0x00;
		cmd[6] = (uint8_t)(size >> 24);
		cmd[7] = (uint8_t)((size >> 16) & 0xff);
		cmd[8] = (uint8_t)((size >> 8) & 0xff);
		cmd[9] = (uint8_t)(size & 0xff);
		cmd[10] = 0x00;
		cmd[11] = 0x00;
		break;

	case UFS_OP_SECURITY_PROTOCOL_OUT:
		cmd[0] = UFS_OP_SECURITY_PROTOCOL_OUT;
		cmd[1] = SECURITY_PROTOCOL;
		if (dev_ver >= RPMB_VER_UFS_30)
			cmd[2] = region;
		else
			cmd[2] = 0;
		cmd[3] = 0x01;
		cmd[4] = 0x00;
		cmd[5] = 0x00;
		cmd[6] = (uint8_t)((size >> 24));
		cmd[7] = (uint8_t)((size >> 16) & 0xff);
		cmd[8] = (uint8_t)((size >> 8) & 0xff);
		cmd[9] = (uint8_t)(size & 0xff);
		cmd[10] = 0x00;
		cmd[11] = 0x00;
		break;

	default:
		break;
	}
}

static inline void ufs_rpmb_combine_cmd(struct combine_cmd *cmd, uint8_t region)
{
	/*the scsi SG_IO header*/
	struct sg_io_v4 *siv;
	struct ufs_blk_ioc_data *data = cmd->data;

	siv = &data->siv;

	siv->guard = 'Q';
	siv->protocol = BSG_PROTOCOL_SCSI;
	siv->subprotocol = BSG_SUB_PROTOCOL_SCSI_CMD;
	siv->response = (uint64_t)cmd->sense_buffer;
	siv->max_response_len = MAX_SENSE_BUFFER_LENGTH;
	siv->request_len = SCSI_RPMB_COMMAND_LENGTH;
	if (cmd->is_write) {
		ufs_get_cdb_rpmb_command(UFS_OP_SECURITY_PROTOCOL_OUT,
					 (uint32_t)data->buf_bytes,
					 cmd->sdb_command, region);
		siv->dout_xfer_len = (uint32_t)cmd->blocks * cmd->blksz;
		siv->dout_xferp = (uint64_t)cmd->transfer_frame;
		siv->request = (__u64)cmd->sdb_command;
	} else {
		ufs_get_cdb_rpmb_command(UFS_OP_SECURITY_PROTOCOL_IN,
					 (uint32_t)data->buf_bytes,
					 cmd->sdb_command, region);
		siv->din_xfer_len = (uint32_t)cmd->blocks * cmd->blksz;
		siv->din_xferp = (uint64_t)cmd->transfer_frame;
		siv->request = (__u64)cmd->sdb_command;
	}
}

static inline void ufs_rpmb_combine_request(struct combine_cmd *cmd,
					    uint8_t region)
{
	struct ufs_blk_ioc_data *data = cmd->data;

	/*the scsi SG_IO header*/
	data->buf = (void *)cmd->frame;
	data->buf_bytes = (u64)cmd->blocks * cmd->blksz;
	if (data->buf_bytes > sizeof(struct rpmb_frame) * MAX_RPMB_FRAME) {
		pr_err("[%s]:buf_bytes outside transfer_frame\n", __func__);
		return;
	}
	memcpy(cmd->transfer_frame, cmd->frame, data->buf_bytes); /* unsafe_function_ignore: memcpy */ /*[false alarm]: enough reserved memory, 32KB*/
	ufs_rpmb_combine_cmd(cmd, region);
}

void ufs_rpmb_set_key(struct rpmb_request *req,
		      struct ufs_blk_ioc_rpmb_data *rpmb_data,
		      uint8_t *sense_buffer[],
		      struct rpmb_frame *transfer_frame[])
{
	int i;
	bool is_write;
	struct rpmb_frame *frame;
	struct combine_cmd cmd;
	uint8_t region = req->info.rpmb_region_num;
	/*according to key frame request, caculate the status request*/
	uint16_t status_frame_request_type =
		(uint16_t)(((be16_to_cpu(req->key_frame.request)) & 0xF000) |
			   RPMB_REQ_STATUS);

	memset(&req->status_frame, 0, sizeof(struct rpmb_frame)); /* unsafe_function_ignore: memset */
	req->status_frame.request = cpu_to_be16(status_frame_request_type);

	/* include basic request(0), status(1), result(2) frmaes */
	for (i = 0; i < 3; i++) {
		/* result frame (index 2) is read */
		is_write = (i == 2) ? false : true;
		frame = (i == 1) ? &req->status_frame : &req->key_frame;
		CMD_PARA_SET(&cmd, &rpmb_data->data[i], frame, is_write,
			     0x1, RPMB_BLK_SZ,
			     (u8 *)&rpmb_data->sdb_command[i], sense_buffer[i],
			     transfer_frame[i]);
		ufs_rpmb_combine_request(&cmd, region);
	}
}

void ufs_rpmb_read_data(struct rpmb_request *req,
			struct ufs_blk_ioc_rpmb_data *rpmb_data,
			uint8_t *sense_buffer[],
			struct rpmb_frame *transfer_frame[])
{
	int i;
	bool is_write;
	unsigned short blks;
	struct combine_cmd cmd;
	uint8_t region = req->info.rpmb_region_num;
	/*get read total blocks*/
	unsigned short blocks_count = (uint16_t)req->info.current_rqst.blks;

	/* include basic request(0), result(1) frmaes */
	for (i = 0; i < 2; i++) {
		is_write = (i == 1) ? false : true;
		blks = (i == 1) ? blocks_count : 0x1;
		CMD_PARA_SET(&cmd, &rpmb_data->data[i], &req->frame[0], is_write,
			     blks, RPMB_BLK_SZ,
			     (u8 *)&rpmb_data->sdb_command[i], sense_buffer[i],
			     transfer_frame[i]);
		ufs_rpmb_combine_request(&cmd, region);
	}
}

void ufs_rpmb_get_counter(struct rpmb_request *req,
			  struct ufs_blk_ioc_rpmb_data *rpmb_data,
			  uint8_t *sense_buffer[],
			  struct rpmb_frame *transfer_frame[])
{
	int i;
	bool is_write;
	struct combine_cmd cmd;
	uint8_t region = req->info.rpmb_region_num;

	/* include basic request(0), result(1) frmaes */
	for (i = 0; i < 2; i++) {
		/* result frame (index 1) is read */
		is_write = (i == 1) ? false : true;
		CMD_PARA_SET(&cmd, &rpmb_data->data[i], &req->frame[0], is_write,
			     0x1, RPMB_BLK_SZ,
			     (u8 *)&rpmb_data->sdb_command[i], sense_buffer[i],
			     transfer_frame[i]);
		ufs_rpmb_combine_request(&cmd, region);
	}
}

void ufs_rpmb_write_data(struct rpmb_request *req,
			 struct ufs_blk_ioc_rpmb_data *rpmb_data,
			 uint8_t *sense_buffer[],
			 struct rpmb_frame *transfer_frame[])
{
	int i;
	bool is_write;
	unsigned short blks;
	struct rpmb_frame *frame;
	struct combine_cmd cmd;
	uint8_t region = req->info.rpmb_region_num;
	/*get write total blocks*/
	unsigned short blocks_count =
		(unsigned short)req->info.current_rqst.blks;
	/*according to write frame request, caculate the status request*/
	uint16_t status_frame_request_type =
		(uint16_t)(((be16_to_cpu(req->frame[0].request)) & 0xF000) |
			   RPMB_REQ_STATUS);

	memset(&req->status_frame, 0, sizeof(struct rpmb_frame)); /* unsafe_function_ignore: memset */
	req->status_frame.request = cpu_to_be16(status_frame_request_type);

	/* include basic request(0), status(1), result(2) frmaes */
	for (i = 0; i < 3; i++) {
		frame = (i == 1) ? &req->status_frame : &req->frame[0];
		/* result frame (index 2) is read */
		is_write = (i == 2) ? false : true;
		blks = (i == 0) ? blocks_count : 0x1;
		CMD_PARA_SET(&cmd, &rpmb_data->data[i], frame, is_write,
			     blks, RPMB_BLK_SZ,
			     (u8 *)&rpmb_data->sdb_command[i], sense_buffer[i],
			     transfer_frame[i]);
		ufs_rpmb_combine_request(&cmd, region);
	}
}

static int ufs_resoure_alloc(struct ufs_blk_ioc_rpmb_data **rpmb_data,
			     struct rpmb_frame *transfer_frame[],
			     uint8_t *sense_buffer[])
{
	int i;
	int tran_index = -1;
	int sense_index = -1;

	*rpmb_data = kzalloc(sizeof(struct ufs_blk_ioc_rpmb_data), GFP_KERNEL);
	if (!*rpmb_data) {
		pr_err("alloc rpmb_data failed\n");
		return RPMB_ERR_MEMALOC;
	}

	for (i = 0; i < UFS_IOC_MAX_RPMB_CMD; i++) {
		transfer_frame[i] = kzalloc(
			MAX_RPMB_FRAME * sizeof(struct rpmb_frame), GFP_KERNEL);
		if (!transfer_frame[i]) {
			pr_err("alloc rpmb transfer_frame failed\n");
			goto free_transfer;
		}
		tran_index = i;
	}

	for (i = 0; i < UFS_IOC_MAX_RPMB_CMD; i++) {
		sense_buffer[i] = kcalloc(
			MAX_SENSE_BUFFER_LENGTH, sizeof(uint8_t), GFP_KERNEL);
		if (!sense_buffer[i]) {
			pr_err("alloc rpmb sense_buffer failed\n");
			goto free_sense;
		}
		sense_index = i;
	}

	return 0;

free_sense:
	for (i = 0; i <= sense_index; i++)
		kfree(sense_buffer[i]);
free_transfer:
	for (i = 0; i <= tran_index; i++)
		kfree(transfer_frame[i]);

	kfree(*rpmb_data);
	return RPMB_ERR_MEMALOC;
}

static void ufs_resoure_free(struct ufs_blk_ioc_rpmb_data *rpmb_data,
			     struct rpmb_frame *transfer_frame[],
			     uint8_t *sense_buffer[])
{
	int i;

	kfree(rpmb_data);
	rpmb_data = NULL;
	for (i = 0; i < UFS_IOC_MAX_RPMB_CMD; i++) {
		kfree(transfer_frame[i]);
		kfree(sense_buffer[i]);
		transfer_frame[i] = NULL;
		sense_buffer[i] = NULL;
	}
}

/*lint -e454 -e456*/
static int ufs_rpmb_work(struct rpmb_request *request)
{
	int result;

	struct rpmb_frame *transfer_frame[UFS_IOC_MAX_RPMB_CMD] = { NULL };
	uint8_t *sense_buffer[UFS_IOC_MAX_RPMB_CMD] = { NULL };
	struct rpmb_frame *frame = &request->frame[0];
	struct ufs_blk_ioc_rpmb_data *rpmb_data = NULL;

	/* ufs block ioctl rpmb data, include ufs scsi packet -- sg_io_hdr */

	result = ufs_resoure_alloc(&rpmb_data, transfer_frame, sense_buffer);
	if (result)
		return result;

	switch (request->info.state) {
	case RPMB_STATE_IDLE:
		pr_err("[%s]:rpmb maybe issue an error\n", __func__);
		break;
	case RPMB_STATE_KEY:
		frame = &request->key_frame;
		ufs_rpmb_set_key(request, rpmb_data, sense_buffer,
				 transfer_frame);
		break;
	case RPMB_STATE_RD:
		ufs_rpmb_read_data(request, rpmb_data, sense_buffer,
				   transfer_frame);
		break;
	case RPMB_STATE_CNT:
		ufs_rpmb_get_counter(request, rpmb_data, sense_buffer,
				     transfer_frame);
		break;
	case RPMB_STATE_WR_CNT:
		/* TODO add a lock here for counter before write data */
		mutex_lock(&rpmb_counter_lock);
		ufs_rpmb_get_counter(request, rpmb_data, sense_buffer,
				     transfer_frame);
		break;
	case RPMB_STATE_WR_DATA:
		ufs_rpmb_write_data(request, rpmb_data, sense_buffer,
				    transfer_frame);
		/* TODO add a unlock for counter after write data */
		break;
	}
	result = ufs_bsg_ioctl_rpmb_cmd(RPMB_FUNC_ID_SE, rpmb_data);

	/*
	 * we must unlock rpmb_counter_lock for some condition
	 * 1. RPMB_STATE_WR_CNT and result failed
	 * 2. RPMB_STATE_WR_CNT and result success but RESPONSE not valid
	 * 3. RPMB_STATE_WR_DATA we always unlock
	 */
	if ((request->info.state == RPMB_STATE_WR_CNT &&
	     (result || (be16_to_cpu(frame->result) != RPMB_OK ||
			 be16_to_cpu(frame->request) != RPMB_RESP_WCOUNTER))) ||
	    (request->info.state == RPMB_STATE_WR_DATA)) {
		mutex_unlock(&rpmb_counter_lock);
	}

	ufs_resoure_free(rpmb_data, transfer_frame, sense_buffer);
	return result;
}
/*lint +e454 +e456*/

static ssize_t ufs_rpmb_key_store(struct device *dev,
				  struct rpmb_request *req)
{
	int i, ret = 0;
	struct rpmb_request *request = req;
	struct rpmb_frame *frame = &request->key_frame;

	struct rpmb_frame *transfer_frame[UFS_IOC_MAX_RPMB_CMD] = { NULL };
	uint8_t *sense_buffer[UFS_IOC_MAX_RPMB_CMD] = { NULL };
	struct ufs_blk_ioc_rpmb_data *rpmb_data = NULL;

	ret = ufs_resoure_alloc(&rpmb_data, transfer_frame, sense_buffer);
	if (ret)
		return ret;

	/* before set key must init the request */
	request->key_frame_status = KEY_NOT_READY;
	memset(frame, 0, sizeof(struct rpmb_frame)); /* unsafe_function_ignore: memset */

	/* get key from bl31 */
	ret = atfd_hisi_rpmb_smc((u64)RPMB_SVC_SET_KEY, (u64)0x0, (u64)0x0,
				 (u64)0x0);
	if (ret) {
		dev_err(dev, "get rpmb key frame failed, ret = %d\n", ret);
		goto alloc_free;
	}

	for (i = 0; i < WAIT_KEY_FRAME_TIMES; i++) {
		if (request->key_frame_status == KEY_READY)
			break;
		mdelay(5);
	}
	if (i == WAIT_KEY_FRAME_TIMES) {
		dev_err(dev, "ufs wait for key frame ready timeout\n");
		goto alloc_free;
	}

	ufs_rpmb_set_key(request, rpmb_data, sense_buffer, transfer_frame);
	ret = ufs_bsg_ioctl_rpmb_cmd(RPMB_FUNC_ID_SE, rpmb_data);

alloc_free:
	ufs_resoure_free(rpmb_data, transfer_frame, sense_buffer);

	if (ret || (be16_to_cpu(frame->result) != RPMB_OK ||
		    be16_to_cpu(frame->request) != RPMB_RESP_KEY)) {
		pr_err("set ufs emmc single key failed\n");
		rpmb_print_frame_buf("error frame", (void *)frame, 512, 16);
		return -1;
	}
	return ret;
}

/* check the rpmb key in ufs is OK */
static int32_t ufs_rpmb_key_status(void)
{
	int ret;
	struct rpmb_request *request;
	struct rpmb_frame *frame;
	struct rpmb_frame *transfer_frame[UFS_IOC_MAX_RPMB_CMD] = { NULL };
	uint8_t *sense_buffer[UFS_IOC_MAX_RPMB_CMD] = { NULL };
	struct ufs_blk_ioc_rpmb_data *rpmb_data = NULL;

	request = kzalloc(sizeof(struct rpmb_request), GFP_KERNEL);
	if (!request) {
		pr_err("[%s]:alloc rpmb_request failed\n", __func__);
		return RPMB_ERR_MEMALOC;
	}

	ret = ufs_resoure_alloc(&rpmb_data, transfer_frame, sense_buffer);
	if (ret) {
		kfree(request);
		return ret;
	}

	frame = &request->frame[0];
	memset(frame, 0, sizeof(struct rpmb_frame)); /* unsafe_function_ignore: memset */
	frame->request = cpu_to_be16(RPMB_REQ_WCOUNTER);
	ufs_rpmb_get_counter(request, rpmb_data, sense_buffer, transfer_frame);
	ret = ufs_bsg_ioctl_rpmb_cmd(RPMB_FUNC_ID_SE, rpmb_data);
	if (ret) {
		pr_err("[%s]:can not get rpmb key status\n", __func__);
		goto alloc_free;
	}

	if (be16_to_cpu(frame->result) == RPMB_ERR_KEY &&
	    be16_to_cpu(frame->request) == RPMB_RESP_WCOUNTER) {
		pr_err("[%s]:RPMB KEY is not set\n", __func__);
		ret = RPMB_ERR_KEY;
	} else if (be16_to_cpu(frame->result) != RPMB_OK ||
		   be16_to_cpu(frame->request) != RPMB_RESP_WCOUNTER) {
		pr_err("[%s]:get write counter failed\n", __func__);
		rpmb_print_frame_buf("error frame", (void *)frame, 512, 16);
		ret = RPMB_ERR_GET_COUNT;
	} else
		ret = RPMB_OK;

alloc_free:
	ufs_resoure_free(rpmb_data, transfer_frame, sense_buffer);
	kfree(request);
	return ret;
}

#define FRAME_BLOCK_COUNT 506
static inline void
hisi_ufs_read_frame_recombine(struct storage_blk_ioc_rpmb_data *storage_data)
{
	unsigned int block_count;

	/* The blocks is one request cmd that needs to transfer(read or write)
	 * the frames, this is set by SECURE OS. Because read rpmb need two
	 * request cmd and the second request includes the block_count that
	 * SECURE OS really needs to read, the first request's  frame's
	 * block_count in ufs rpmb needs to be set before sending cmd to
	 * device. */
	block_count = storage_data->data[1].blocks;
	/*num of rpmb blks MSB*/
	storage_data->data[0].buf[FRAME_BLOCK_COUNT] =
		(uint8_t)((block_count & 0xFF00) >> 8);
	/*num of rpmb blks MSB*/
	storage_data->data[0].buf[FRAME_BLOCK_COUNT + 1] =
		(uint8_t)(block_count & 0xFF);
	return;
}

static void
ufs_rpmb_ioctl_combine_request(enum rpmb_op_type operation,
			       struct ufs_blk_ioc_rpmb_data *rpmb_data,
			       struct storage_blk_ioc_rpmb_data *storage_data,
			       uint8_t *sense_buffer[])
{
	int i;
	int frame_cnt = 0;
	int write_flag[3] = { 0 };
	struct combine_cmd cmd;

	switch (operation) {
	case RPMB_OP_RD:
		/* include basic request(0), result(1) frmaes */
		frame_cnt = 2;
		write_flag[0] = 1;
		break;
	case RPMB_OP_WR_CNT:
		/* include basic request(0), result(1) frmaes */
		frame_cnt = 2;
		write_flag[0] = 1;
		break;
	case RPMB_OP_WR_DATA:
		/* include basic request(0), status(1), result(2) frmaes */
		frame_cnt = 3;
		write_flag[0] = 1;
		write_flag[1] = 1;
		break;
	}

	for (i = 0; i < frame_cnt; i++) {
		cmd.data = &rpmb_data->data[i];
		/* result frame (index 1) is read */
		cmd.is_write = write_flag[i];
		cmd.blocks = (unsigned short)storage_data->data[i].blocks;
		cmd.blksz = RPMB_BLK_SZ;
		cmd.sdb_command = (u8 *)&rpmb_data->sdb_command[i];
		cmd.sense_buffer = sense_buffer[i];
		cmd.transfer_frame =
			(struct rpmb_frame *)rpmb_data->data[i].buf;
		ufs_rpmb_combine_cmd(&cmd, 0);
	}

}

int hisi_ufs_rpmb_ioctl_cmd(enum func_id id, enum rpmb_op_type operation,
			    struct storage_blk_ioc_rpmb_data *storage_data)
{
	int ret;
	int i;
	uint8_t *sense_buffer[UFS_IOC_MAX_RPMB_CMD] = { NULL };
	struct rpmb_frame *transfer_frame[UFS_IOC_MAX_RPMB_CMD] = { NULL };
	struct ufs_blk_ioc_rpmb_data *rpmb_data = NULL;

	ret = ufs_resoure_alloc(&rpmb_data, transfer_frame, sense_buffer);
	if (ret)
		return ret;

	if (RPMB_OP_RD == operation)
		hisi_ufs_read_frame_recombine(storage_data);

	for (i = 0; i < STORAGE_IOC_MAX_RPMB_CMD; i++) {
		rpmb_data->data[i].buf_bytes = storage_data->data[i].buf_bytes;
		/* when Secure OS write multi blocks in HYNIX rpmb, it will
		 * timeout, memcpy to avoid the error */
		if (UFS_VENDOR_HYNIX == get_bootdevice_manfid()) {
			rpmb_data->data[i].buf = kzalloc(
				rpmb_data->data[i].buf_bytes, GFP_KERNEL);
			if (NULL == rpmb_data->data[i].buf) {
				pr_err("[%s]:alloc rpmb_data buf failed\n",
				       __func__);
				ret = RPMB_ERR_MEMALOC;
				goto free_alloc_buf;
			}
			memcpy(rpmb_data->data[i].buf, /* unsafe_function_ignore: memcpy */
			       storage_data->data[i].buf,
			       rpmb_data->data[i].buf_bytes);
		} else
			rpmb_data->data[i].buf = storage_data->data[i].buf;
	}

	ufs_rpmb_ioctl_combine_request(operation, rpmb_data, storage_data,
				       sense_buffer);
	ret = ufs_bsg_ioctl_rpmb_cmd(id, rpmb_data);

	/* when Secure OS write multi blocks in HYNIX rpmb, it will timeout,
	 * memcpy to avoid the error*/
	if (UFS_VENDOR_HYNIX == get_bootdevice_manfid()) {
		for (i = 0; i < STORAGE_IOC_MAX_RPMB_CMD; i++) {
			memcpy(storage_data->data[i].buf, /* unsafe_function_ignore: memcpy */
			       rpmb_data->data[i].buf,
			       rpmb_data->data[i].buf_bytes);
		}
	}

free_alloc_buf:
	if (UFS_VENDOR_HYNIX == get_bootdevice_manfid()) {
		for (i = 0; i < STORAGE_IOC_MAX_RPMB_CMD; i++) {
			if (rpmb_data->data[i].buf != NULL)
				kfree(rpmb_data->data[i].buf);
		}
	}
	ufs_resoure_free(rpmb_data, transfer_frame, sense_buffer);

	return ret;
}

struct rpmb_operation ufs_rpmb_ops = {
	.issue_work = ufs_rpmb_work,
	.ioctl = hisi_ufs_rpmb_ioctl_cmd,
	.key_store = ufs_rpmb_key_store,
	.key_status = ufs_rpmb_key_status,

};

int rpmb_ufs_init(void)
{
	return rpmb_operation_register(&ufs_rpmb_ops, BOOT_DEVICE_UFS);
}
