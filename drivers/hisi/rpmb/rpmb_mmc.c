

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/syscalls.h>

#include <linux/bootdevice.h>
#include <linux/mmc/core.h>
#include <linux/mmc/ioctl.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/hisi/rpmb.h>

#include <asm/uaccess.h>

#include "hisi_rpmb.h"


static inline void mmc_rpmb_combine_cmd(struct mmc_blk_ioc_data *data,
					uint32_t is_write,
					unsigned short blocks,
					unsigned short blksz, uint32_t reliable)
{
	struct mmc_ioc_cmd *ioc;

	ioc = &data->ic;
	ioc->write_flag = (int)(is_write | ((is_write & reliable) << 31));
	ioc->opcode =
		is_write ? MMC_WRITE_MULTIPLE_BLOCK : MMC_READ_MULTIPLE_BLOCK;
	ioc->flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	ioc->arg = 0x0;
	ioc->blksz = blksz;
	ioc->blocks = blocks;
}

static inline void mmc_rpmb_combine_request(struct mmc_blk_ioc_data *data,
					    uint32_t is_write,
					    unsigned short blocks,
					    unsigned short blksz,
					    uint32_t reliable)
{
	data->buf_bytes = (u64)blocks * blksz;
	mmc_rpmb_combine_cmd(data, is_write, blocks, blksz, reliable);
}

static inline void
mmc_rpmb_basic_request(uint32_t index, struct rpmb_frame *frame,
		       struct mmc_blk_ioc_rpmb_data *rpmb_data,
		       uint16_t block_count, uint32_t reliable)
{
	rpmb_data->data[index].buf = (void *)frame;
	mmc_rpmb_combine_request(&rpmb_data->data[index], true,
				 block_count, RPMB_BLK_SZ, reliable);
}

static inline void
mmc_rpmb_status_request(uint32_t index, struct rpmb_frame *frame,
			struct mmc_blk_ioc_rpmb_data *rpmb_data,
			uint16_t block_count, uint32_t reliable)
{
	rpmb_data->data[index].buf = (void *)frame;
	mmc_rpmb_combine_request(&rpmb_data->data[index], true,
				 block_count, RPMB_BLK_SZ, reliable);
}

static inline void
mmc_rpmb_result_request(uint32_t index, struct rpmb_frame *frame,
			struct mmc_blk_ioc_rpmb_data *rpmb_data,
			uint16_t block_count, uint32_t reliable)
{
	rpmb_data->data[index].buf = (void *)frame;
	mmc_rpmb_combine_request(&rpmb_data->data[index], false,
				 block_count, RPMB_BLK_SZ, reliable);
}

void mmc_rpmb_set_key(struct rpmb_request *req,
		      struct mmc_blk_ioc_rpmb_data *rpmb_data)
{
	/* according to key frame request, caculate the status request */
	uint16_t status_frame_request_type =
		(uint16_t)(((be16_to_cpu(req->key_frame.request)) & 0xF000) |
			   RPMB_REQ_STATUS);

	memset(&req->status_frame, 0, sizeof(struct rpmb_frame)); /* unsafe_function_ignore: memset */
	req->status_frame.request = cpu_to_be16(status_frame_request_type);
	mmc_rpmb_basic_request(0, &req->key_frame, rpmb_data, 0x1, true);
	mmc_rpmb_status_request(1, &req->status_frame, rpmb_data, 0x1, false);
	mmc_rpmb_result_request(2, &req->key_frame, rpmb_data, 0x1, false);
}

void mmc_rpmb_get_counter(struct rpmb_request *req,
			  struct mmc_blk_ioc_rpmb_data *rpmb_data)
{
	mmc_rpmb_basic_request(0, &req->frame[0], rpmb_data, 0x1, false);
	mmc_rpmb_result_request(1, &req->frame[0], rpmb_data, 0x1, false);
}

void mmc_rpmb_read_data(struct rpmb_request *req,
			struct mmc_blk_ioc_rpmb_data *rpmb_data)
{
	/* get read total blocks */
	unsigned short blocks_count = (uint16_t)req->info.current_rqst.blks;

	mmc_rpmb_basic_request(0, &req->frame[0], rpmb_data, 0x1, false);
	mmc_rpmb_result_request(1, &req->frame[0], rpmb_data, blocks_count,
				false);
}

void mmc_rpmb_write_data(struct rpmb_request *req,
			 struct mmc_blk_ioc_rpmb_data *rpmb_data)
{
	/* get write total blocks */
	unsigned short blocks_count = (uint16_t)req->info.current_rqst.blks;
	/* according to write frame request, caculate the status request  */
	uint16_t status_frame_request_type =
		(uint16_t)(((be16_to_cpu(req->frame[0].request)) & 0xF000) |
			   RPMB_REQ_STATUS);

	memset(&req->status_frame, 0, sizeof(struct rpmb_frame)); /* unsafe_function_ignore: memset */
	req->status_frame.request = cpu_to_be16(status_frame_request_type);
	mmc_rpmb_basic_request(0, &req->frame[0], rpmb_data, blocks_count,
			       true);
	mmc_rpmb_status_request(1, &req->status_frame, rpmb_data, 0x1, false);
	mmc_rpmb_result_request(2, &req->frame[0], rpmb_data, 0x1, false);
}

/*
 * we must unlock rpmb_counter_lock for some condition
 * 1. RPMB_STATE_WR_CNT and result failed
 * 2. RPMB_STATE_WR_CNT and result success but RESPONSE not valid
 * 3. RPMB_STATE_WR_DATA and all the emmc blocks have been written
 */
void emmc_rpmb_unlock_counterlock(struct rpmb_request *req,
				  struct rpmb_frame *frame, int32_t result)
{
	if ((req->info.state == RPMB_STATE_WR_CNT &&
	     (result || (be16_to_cpu(frame->result) != RPMB_OK ||
			 be16_to_cpu(frame->request) != RPMB_RESP_WCOUNTER))) ||
	    (req->info.state == RPMB_STATE_WR_DATA &&
	     (0 ==
	      req->info.blks - (req->info.current_rqst.offset +
				req->info.current_rqst.blks)))) {
		mutex_unlock(&rpmb_counter_lock); /*lint !e455*/
	}
}

/*
 * This warning describes the "lock" exception used in
 * the function, but according to the code The "lock"
 * added to the function is normal and no additional
 * modifications are required
 */
/*lint -e454 -e456*/
static int32_t mmc_rpmb_work(struct rpmb_request *request)
{
	int32_t result;
	struct block_device *bdev;
	struct mmc_blk_ioc_rpmb_data *rpmb_data;
	struct rpmb_frame *frame = &request->frame[0];

	bdev = blkdev_get_by_path(EMMC_RPMB_BLOCK_DEVICE_NAME,
				  FMODE_READ | FMODE_WRITE | FMODE_NDELAY, 0);
	if (IS_ERR(bdev)) {
		pr_err("[%s]:HISEE open rpmb block failed!\n", __func__);
		return RPMB_ERR_BLKDEV;
	}

	rpmb_data = kzalloc(sizeof(*rpmb_data), GFP_KERNEL);
	if (rpmb_data == NULL) {
		pr_err("[%s]:alloc rpmb_data failed\n", __func__);
		return RPMB_ERR_MEMALOC;
	}

	switch (request->info.state) {
	case RPMB_STATE_IDLE:
		pr_err("[%s]:rpmb maybe issue an error\n", __func__);
		break;
	case RPMB_STATE_KEY:
		frame = &request->key_frame;
		mmc_rpmb_set_key(request, rpmb_data);
		break;
	case RPMB_STATE_RD:
		mmc_rpmb_read_data(request, rpmb_data);
		break;
	case RPMB_STATE_CNT:
		mmc_rpmb_get_counter(request, rpmb_data);
		break;
	case RPMB_STATE_WR_CNT:
		/* TODO add a lock here for counter before write data */
		mutex_lock(&rpmb_counter_lock);
		mmc_rpmb_get_counter(request, rpmb_data);
		break;
	case RPMB_STATE_WR_DATA:
		mmc_rpmb_write_data(request, rpmb_data);
		/* TODO add a unlock for counter after write data */
		break;
	}

	result = mmc_blk_ioctl_rpmb_cmd((enum func_id)request->info.func_id,
					bdev, rpmb_data);

	emmc_rpmb_unlock_counterlock(request, frame, result);
	if (bdev)
		blkdev_put(bdev, FMODE_READ | FMODE_WRITE | FMODE_NDELAY);
	kfree(rpmb_data);

	return result;
}
/*lint +e454 +e456*/

static ssize_t mmc_rpmb_key_store(struct device *dev,
				  struct rpmb_request *req)
{
	int ret = 0;
	int i;
	struct rpmb_request *request = req;
	struct rpmb_frame *frame = &request->key_frame;
	struct mmc_blk_ioc_rpmb_data *rpmb_data;
	struct block_device *bdev;

	rpmb_data = kzalloc(sizeof(*rpmb_data), GFP_KERNEL);
	if (!rpmb_data) {
		dev_err(dev, "alloc rpmb_data failed\n");
		return -1;
	}

	/* before set key must init the request */
	request->key_frame_status = KEY_NOT_READY;
	memset(frame, 0, sizeof(struct rpmb_frame)); /* unsafe_function_ignore: memset */


	bdev = blkdev_get_by_path(EMMC_RPMB_BLOCK_DEVICE_NAME,
				  FMODE_READ | FMODE_WRITE | FMODE_NDELAY, 0);
	if (IS_ERR(bdev)) {
		dev_err(dev, "blkdev get mmcblk0rpmb failed\n");
		goto alloc_free;
	}

	/* get key from bl31 */
	ret = atfd_hisi_rpmb_smc((u64)RPMB_SVC_SET_KEY, (u64)0x0, (u64)0x0,
				 (u64)0x0);
	if (ret) {
		dev_err(dev, "get rpmb key frame failed, ret %d\n", ret);
		goto alloc_free;
	}

	for (i = 0; i < WAIT_KEY_FRAME_TIMES; i++) {
		if (request->key_frame_status == KEY_READY)
			break;
		mdelay(5);
	}
	if (i == WAIT_KEY_FRAME_TIMES) {
		dev_err(dev, "mmc wait for key frame ready timeout\n");
		goto alloc_free;
	}


	mmc_rpmb_set_key(request, rpmb_data);
	ret = mmc_blk_ioctl_rpmb_cmd((enum func_id)request->info.func_id,
				     bdev, rpmb_data);

	blkdev_put(bdev, FMODE_READ | FMODE_WRITE | FMODE_NDELAY);
alloc_free:
	if (rpmb_data != NULL)
		kfree(rpmb_data);
	if (ret || (be16_to_cpu(frame->result) != RPMB_OK ||
		    be16_to_cpu(frame->request) != RPMB_RESP_KEY)) {
		pr_err("set emmc rpmb single key failed\n");
		rpmb_print_frame_buf("error frame", (void *)frame, 512, 16);
		return -1;
	}

	return ret;
}

/* check the rpmb key in emmc is OK */
static int32_t mmc_rpmb_key_status(void)
{
	int ret;
	struct rpmb_request *request;
	struct rpmb_frame *frame;
	struct block_device *blkdev;
	struct mmc_blk_ioc_rpmb_data *rpmb_data;

	request = kzalloc(sizeof(struct rpmb_request), GFP_KERNEL);
	if (!request) {
		pr_err("[%s]:alloc rpmb_request failed\n", __func__);
		return RPMB_ERR_MEMALOC;
	}

	frame = &request->frame[0];
	rpmb_data = kzalloc(sizeof(*rpmb_data), GFP_KERNEL);
	if (!rpmb_data) {
		pr_err("[%s]:alloc rpmb_data failed.\n", __func__);
		ret = RPMB_ERR_MEMALOC;
		goto alloc_free;
	}

	blkdev = blkdev_get_by_path(EMMC_RPMB_BLOCK_DEVICE_NAME,
				  FMODE_READ | FMODE_WRITE | FMODE_NDELAY, 0);
	if (IS_ERR(blkdev)) {
		pr_err("[%s]:rpmb device get failed\n", __func__);
		ret = RPMB_ERR_BLKDEV;
		goto alloc_free;
	}

	memset(frame, 0, sizeof(struct rpmb_frame)); /* unsafe_function_ignore: memset */
	frame->request = cpu_to_be16(RPMB_REQ_WCOUNTER);
	mmc_rpmb_get_counter(request, rpmb_data);

	ret = mmc_blk_ioctl_rpmb_cmd(RPMB_FUNC_ID_SE, blkdev, rpmb_data);
	if (ret) {
		pr_err("[%s]:can not get rpmb key status\n", __func__);
		goto blkdev_put;
	}

	if (be16_to_cpu(frame->result) == RPMB_ERR_KEY &&
	    be16_to_cpu(frame->request) == RPMB_RESP_WCOUNTER) {
		pr_err("[%s]:RPMB KEY is not set\n", __func__);
		ret = RPMB_ERR_SET_KEY;
	} else if (be16_to_cpu(frame->result) != RPMB_OK ||
		   be16_to_cpu(frame->request) != RPMB_RESP_WCOUNTER) {
		pr_err("[%s]:get write counter failed\n", __func__);
		rpmb_print_frame_buf("error frame", (void *)frame, 512, 16);
		ret = RPMB_ERR_GET_COUNT;
	} else {
		ret = RPMB_OK;
	}

blkdev_put:
	if (blkdev)
		blkdev_put(blkdev, FMODE_READ | FMODE_WRITE | FMODE_NDELAY);
alloc_free:
	if (rpmb_data != NULL)
		kfree(rpmb_data);
	if (request != NULL)
		kfree(request);
	return ret;
}

int hisi_mmc_rpmb_ioctl_cmd(enum func_id id, enum rpmb_op_type operation,
			    struct storage_blk_ioc_rpmb_data *storage_data)
{
	struct block_device *bdev;
	int ret;
	int i;
	struct mmc_blk_ioc_rpmb_data *rpmb_data;

	bdev = blkdev_get_by_path(EMMC_RPMB_BLOCK_DEVICE_NAME,
				  FMODE_READ | FMODE_WRITE | FMODE_NDELAY, 0);
	if (IS_ERR(bdev)) {
		pr_err("[%s]:Secure OS open rpmb block failed!\n", __func__);
		return RPMB_ERR_BLKDEV;
	}

	rpmb_data = kzalloc(sizeof(*rpmb_data), GFP_KERNEL);
	if (rpmb_data == NULL) {
		pr_err("[%s]:alloc rpmb_data failed\n", __func__);
		return RPMB_ERR_MEMALOC;
	}

	for (i = 0; i < STORAGE_IOC_MAX_RPMB_CMD; i++) {
		rpmb_data->data[i].buf = storage_data->data[i].buf;
		rpmb_data->data[i].buf_bytes = storage_data->data[i].buf_bytes;
	}

	switch (operation) {
	case RPMB_OP_RD:
		mmc_rpmb_combine_cmd(
			&rpmb_data->data[0], true,
			(unsigned short)storage_data->data[0].blocks,
			RPMB_BLK_SZ, false);
		mmc_rpmb_combine_cmd(
			&rpmb_data->data[1], false,
			(unsigned short)storage_data->data[1].blocks,
			RPMB_BLK_SZ, false);
		break;
	case RPMB_OP_WR_CNT:
		mmc_rpmb_combine_cmd(
			&rpmb_data->data[0], true,
			(unsigned short)storage_data->data[0].blocks,
			RPMB_BLK_SZ, false);
		mmc_rpmb_combine_cmd(
			&rpmb_data->data[1], false,
			(unsigned short)storage_data->data[1].blocks,
			RPMB_BLK_SZ, false);
		break;
	case RPMB_OP_WR_DATA:
		mmc_rpmb_combine_cmd(
			&rpmb_data->data[0], true,
			(unsigned short)storage_data->data[0].blocks,
			RPMB_BLK_SZ, true);
		mmc_rpmb_combine_cmd(
			&rpmb_data->data[1], true,
			(unsigned short)storage_data->data[1].blocks,
			RPMB_BLK_SZ, false);
		mmc_rpmb_combine_cmd(
			&rpmb_data->data[2], false,
			(unsigned short)storage_data->data[2].blocks,
			RPMB_BLK_SZ, false);
		break;
	}

	ret = mmc_blk_ioctl_rpmb_cmd(id, bdev, rpmb_data);

	if (bdev)
		blkdev_put(bdev, FMODE_READ | FMODE_WRITE | FMODE_NDELAY);
	kfree(rpmb_data);

	return ret;
}


struct rpmb_operation mmc_rpmb_ops = {
	.issue_work = mmc_rpmb_work,
	.ioctl = hisi_mmc_rpmb_ioctl_cmd,
	.key_store = mmc_rpmb_key_store,
	.key_status = mmc_rpmb_key_status,

};

int rpmb_mmc_init(void)
{
	return rpmb_operation_register(&mmc_rpmb_ops, BOOT_DEVICE_EMMC);
}
