#include <linux/version.h>
#include <linux/mmc/core.h>
#include <linux/mmc/ioctl.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/hisi/rpmb.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include "core.h"
#endif
#include "mmc_hisi_card.h"

static struct mmc_blk_ioc_rpmb_data *
mmc_blk_ioctl_rpmb_copy_data(struct mmc_blk_ioc_rpmb_data *rdata)
{
	struct mmc_blk_ioc_rpmb_data *idata;
	long err;
	int i;

	idata = kzalloc(sizeof(*idata), GFP_KERNEL);
	if (!idata) {
		err = -ENOMEM;
		return ERR_PTR(err);
	}

	for (i = 0; i < MMC_IOC_MAX_RPMB_CMD; i++) {
		idata->data[i].buf_bytes = rdata->data[i].buf_bytes;
		idata->data[i].buf =
			kzalloc(idata->data[i].buf_bytes, GFP_KERNEL);
		if (!(idata->data[i].buf)) {
			err = -ENOMEM;
			goto alloc_failed;
		}
		memcpy(&idata->data[i].ic, &rdata->data[i].ic, /* unsafe_function_ignore: memcpy */
		       sizeof(struct mmc_ioc_cmd));
		memcpy(idata->data[i].buf, rdata->data[i].buf, /* unsafe_function_ignore: memcpy */
		       idata->data[i].buf_bytes);
	}

	return idata;

alloc_failed:
	for (i = 0; i < MMC_IOC_MAX_RPMB_CMD; i++) {
		if (idata->data[i].buf != NULL)
			kfree(idata->data[i].buf);
	}
	if (idata != NULL)
		kfree(idata);
	return ERR_PTR(err);
}

/*output api ++*/
struct mmc_card *get_mmc_card(struct block_device *bdev)
{
	struct mmc_blk_data *md;
	struct mmc_card *card;

	md = mmc_blk_get(bdev->bd_disk);
	if (!md)
		return NULL;

	card = md->queue.card;
	if (IS_ERR(card))
		return NULL;

	return card;
}
EXPORT_SYMBOL(get_mmc_card);

static int mmc_rpmb_send_cmd(struct mmc_card *card,
			     struct mmc_blk_ioc_rpmb_data *idata,
			     struct mmc_blk_ioc_rpmb_data *rdata)
{
	struct mmc_command cmd = { 0 };
	struct mmc_data data = { 0 };
	struct mmc_request mrq = { NULL };
	struct scatterlist sg;
	u32 status = 0;
	int err = 0, i = 0;

	for (i = 0; i < MMC_IOC_MAX_RPMB_CMD; i++) {
		struct mmc_blk_ioc_data *curr_data;
		struct mmc_ioc_cmd *curr_cmd;

		curr_data = &idata->data[i];
		curr_cmd = &curr_data->ic;
		if (!curr_cmd->opcode)
			break;

		cmd.opcode = curr_cmd->opcode;
		cmd.arg = curr_cmd->arg;
		cmd.flags = curr_cmd->flags;

		if (curr_data->buf_bytes) {
			data.sg = &sg;
			data.sg_len = 1;
			data.blksz = curr_cmd->blksz;
			data.blocks = curr_cmd->blocks;

			sg_init_one(data.sg, curr_data->buf,
				    (unsigned int)curr_data->buf_bytes);

			if (curr_cmd->write_flag)
				data.flags = MMC_DATA_WRITE;
			else
				data.flags = MMC_DATA_READ;

			/* data.flags must already be set before doing this. */
			mmc_set_data_timeout(&data, card);

			/*
			 * Allow overriding the timeout_ns for empirical tuning.
			 */
			if (curr_cmd->data_timeout_ns)
				data.timeout_ns = curr_cmd->data_timeout_ns;

			mrq.data = &data;
		}

		mrq.cmd = &cmd;

		err = mmc_set_blockcount(
			card, data.blocks,
			(bool)((unsigned int)curr_cmd->write_flag &
			       (1U << 31)));
		if (err)
			goto out;

		mmc_wait_for_req(card->host, &mrq);

		if (cmd.error) {
			dev_err(mmc_dev(card->host), "%s: cmd error %d\n",
				__func__, cmd.error);
			err = (int)cmd.error;
			goto out;
		}
		if (data.error) {
			dev_err(mmc_dev(card->host), "%s: data error %d\n",
				__func__, data.error);
			err = (int)data.error;
			goto out;
		}

		memcpy(curr_cmd->response, cmd.resp, sizeof(cmd.resp)); /* unsafe_function_ignore: memcpy */

		if (!curr_cmd->write_flag) {
			memcpy(rdata->data[i].buf, curr_data->buf, /* unsafe_function_ignore: memcpy */
			       curr_data->buf_bytes);
		}

		/*
		 * Ensure RPMB command has completed by polling CMD13
		 * "Send Status".
		 */
		err = ioctl_rpmb_card_status_poll(card, &status, 5);
		if (err)
			dev_err(mmc_dev(card->host),
				"%s: Card Status=0x%08X, error %d\n", __func__,
				status, err);
	}

out:
	return err;
}


/*This function is responsible for handling RPMB command and is the interface
 *with the eMMC driver.
 *It is used by BL31 and SecureOS.So when modify the function please check it
 *with SecureOS.
 *During DMA 64bit development, we modify it using the method of memory copy.
 *idata:the parameter consist of  two command at least and three commd at most,
 *so when copy retuning
 *      data, please confirm copy all the retuning data not include write
 *command.
 */
/*lint -e429 -e593*/
int mmc_blk_ioctl_rpmb_cmd(enum func_id id, struct block_device *bdev,
			   struct mmc_blk_ioc_rpmb_data *rdata)
{
	struct mmc_blk_data *md;
	struct mmc_card *card;
	struct mmc_blk_ioc_rpmb_data *idata;
	int err = 0, i = 0;
	bool switch_err = false;
	int switch_retry = 3;

	md = mmc_blk_get(bdev->bd_disk);
	/* make sure this is a rpmb partition */
	if ((!md) || (!((unsigned int)md->area_type &
			(unsigned int)MMC_BLK_DATA_AREA_RPMB))) {
		err = -EINVAL;
		return err;
	}

	idata = mmc_blk_ioctl_rpmb_copy_data(rdata);
	if (IS_ERR(idata)) {
		err = (int)PTR_ERR(idata);
		goto cmd_done;
	}

	card = md->queue.card;
	if (IS_ERR(card)) {
		err = (int)PTR_ERR(card);
		goto idata_free;
	}

	mmc_get_card(card);

retry:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	err = mmc_blk_part_switch(card, md->part_type);
#else
	err = mmc_blk_part_switch(card, md);
#endif
	if (err) {
		switch_err = true;
		goto cmd_rel_host;
	}

	err = mmc_rpmb_send_cmd(card, idata, rdata);
	if (err)
		goto cmd_rel_host;

cmd_rel_host:
	if (err == -ENOMSG) {
		if (!mmc_blk_reset(md, card->host, 0)) {
			if (switch_err && switch_retry--) {
				switch_err = false;
				goto retry;
			}
		}
	}

	mmc_put_card(card);

idata_free:
	for (i = 0; i < MMC_IOC_MAX_RPMB_CMD; i++) {
		if (idata->data[i].buf != NULL)
			kfree(idata->data[i].buf);
	}
	if (idata != NULL)
		kfree(idata);

cmd_done:
	mmc_blk_put(md);

	return err;
}
/*lint +e429 +e593*/
