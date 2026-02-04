#include <linux/device.h>
#include <linux/delay.h>
#include <linux/err.h>

#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>

#include "core.h"
#include "bus.h"
#include "host.h"
#include "sdio_bus.h"

#include "mmc_ops.h"
#include "sd_ops.h"
#include "sdio_ops.h"




extern void mmc_set_ios(struct mmc_host *host);
extern void mmc_bus_get(struct mmc_host *host);
extern void mmc_bus_put(struct mmc_host *host);


void mmc_power_up_vcc(struct mmc_host *host,u32 ocr)
{
	if (host->ios.power_mode == MMC_POWER_UP)
		return;

	mmc_host_clk_hold(host);

	host->ios.vdd = fls(ocr) - 1;

	/*
	 * This delay should be sufficient to allow the power supply
	 * to reach the minimum voltage.
	 */
	mmc_delay(10);

	host->ios.clock = host->ios.clock_store ;
	host->ios.power_mode = MMC_POWER_UP;
	mmc_set_ios(host);

	/*
	 * This delay must be at least 74 clock sizes, or 1 ms, or the
	 * time required to reach a stable voltage.
	 */
	mmc_delay(10);

	mmc_host_clk_release(host);
}


void mmc_power_off_vcc(struct mmc_host *host)
{
	if (host->ios.power_mode == MMC_POWER_OFF)
		return;

	mmc_host_clk_hold(host);

	/*store the ios.clock which will be used in mmc_power_up_vcc*/
	host->ios.clock_store = host->ios.clock;
	host->ios.clock = 0;
	host->ios.vdd = 0;

	host->ios.power_mode = MMC_POWER_OFF;

	mmc_set_ios(host);

	/*
	 * Some configurations, such as the 802.11 SDIO card in the OLPC
	 * XO-1.5, require a short delay after poweroff before the card
	 * can be successfully turned on again.
	 */
	mmc_delay(1);

	mmc_host_clk_release(host);
}


int mmc_card_awake(struct mmc_host *host)
{
       int err = -ENOSYS;

       mmc_bus_get(host);

       if (host->bus_ops && !host->bus_dead && host->bus_ops->awake)
               err = host->bus_ops->awake(host);

       mmc_bus_put(host);

       return err;
}
EXPORT_SYMBOL(mmc_card_awake);

int mmc_card_sleep(struct mmc_host *host)
{
       int err = -ENOSYS;

       mmc_bus_get(host);

       if (host->bus_ops && !host->bus_dead && host->bus_ops->sleep)
               err = host->bus_ops->sleep(host);

       mmc_bus_put(host);

       return err;
}
EXPORT_SYMBOL(mmc_card_sleep);

int mmc_card_can_sleep(struct mmc_host *host)
{
       struct mmc_card *card = host->card;

       if (card && mmc_card_mmc(card) && card->ext_csd.rev >= 3)
               return 1;
       return 0;
}
EXPORT_SYMBOL(mmc_card_can_sleep);



/*
 * Turn the cache ON/OFF.
 * Turning the cache OFF shall trigger flushing of the data
 * to the non-volatile storage.
 * This function should be called with host claimed
 */
int mmc_cache_ctrl(struct mmc_host *host, u8 enable)
{
       struct mmc_card *card = host->card;
       unsigned int timeout;
       int err = 0;

       if (!(host->caps2 & MMC_CAP2_CACHE_CTRL) ||
                      mmc_card_is_removable(host))
               return err;

       if (card && mmc_card_mmc(card) &&
                       (card->ext_csd.cache_size > 0)) {
               enable = !!enable;

               if (card->ext_csd.cache_ctrl ^ enable) {
                       timeout = enable ? card->ext_csd.generic_cmd6_time : 0;
                       err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
                                       EXT_CSD_CACHE_CTRL, enable, timeout);
                       if (err)
                               pr_err("%s: cache %s error %d\n",
                                               mmc_hostname(card->host),
                                               enable ? "on" : "off",
                                               err);
                       else
                               card->ext_csd.cache_ctrl = enable;
               }
       }

       return err;
}
EXPORT_SYMBOL(mmc_cache_ctrl);

int mmc_card_sleepawake(struct mmc_host *host, int sleep)
{
	struct mmc_command cmd = {0};
	struct mmc_card *card = host->card;
	int err;
	unsigned long timeout;

	if (sleep)
		(void)mmc_deselect_cards(host);

	cmd.opcode = MMC_SLEEP_AWAKE;
	cmd.arg = card->rca << 16;
	if (sleep)
		cmd.arg |= 1 << 15;

	cmd.flags = MMC_RSP_R1B | MMC_CMD_AC;
	err = mmc_wait_for_cmd(host, &cmd, 0);
	if (err)
		return err;

	/*we use the the check busy method to reduce sr time*/
	if (host->ops->card_busy) {
		timeout = jiffies +
			msecs_to_jiffies(DIV_ROUND_UP(card->ext_csd.sa_timeout, 10000));
		while (host->ops->card_busy(host)) {
			/* Timeout if the device never leaves the program state. */
			if (time_after(jiffies, timeout)) {
				pr_err("%s: wait card not busy time out! %s\n",
				mmc_hostname(host), __func__);
				break;
			}
		}
	} else {
		/*
		 * If the host does not wait while the card signals busy, then we will
		 * will have to wait the sleep/awake timeout.  Note, we cannot use the
		 * SEND_STATUS command to poll the status because that command (and most
		 * others) is invalid while the card sleeps.
		 */
		if (!(host->caps & MMC_CAP_WAIT_WHILE_BUSY))
			mmc_delay(DIV_ROUND_UP(card->ext_csd.sa_timeout, 10000));
	}

	if (!sleep)
		err = mmc_select_card(card);

	return err;
}

void mmc_decode_ext_csd_emmc50(struct mmc_card *card, u8 *ext_csd)
{
		card->ext_csd.raw_sleep_noti_time = ext_csd[EXT_CSD_SLEEP_NOTIFICATION_TIME];
		if (card->ext_csd.raw_sleep_noti_time > 0 && card->ext_csd.raw_sleep_noti_time <= 0x17)
		{
			card->ext_csd.sleep_notification_time = ((unsigned int)1 << card->ext_csd.raw_sleep_noti_time) / 100;/*ms, raw_sleep_noti_time Units: 10us */
			pr_debug("%s: support SLEEP_NOTIFICATION. sleep_notification_time=%d ms\n", mmc_hostname(card->host), card->ext_csd.sleep_notification_time);
		}

#ifdef CONFIG_MMC_CQ_HCI
		card->ext_csd.cmdq_support = ext_csd[EXT_CSD_CMDQ_SUPPORT] & 0x1;
		if (card->ext_csd.cmdq_support) {
			card->ext_csd.cmdq_depth = (ext_csd[EXT_CSD_CMDQ_DEPTH] & 0x1F) + 1;
			pr_err("%s: %s: CMDQ supported: depth: %d\n",
				mmc_hostname(card->host), __func__,
				card->ext_csd.cmdq_depth);
		}
#endif
}

void mmc_decode_ext_csd_emmc51(struct mmc_card *card, u8 *ext_csd)
{

	/* eMMC5.1*/
	if (card->ext_csd.rev >= 8) {
/*strobe*/
		if (ext_csd[EXT_CSD_STROBE_SUPPORT]) {
			card->ext_csd.strobe_enhanced = ext_csd[EXT_CSD_STROBE_SUPPORT];
			if (card->ext_csd.strobe_enhanced)
				pr_info("%s: support STROBE_ENHANCED.\n", mmc_hostname(card->host));
			else
				pr_warn("%s: EXT_CSD_STROBE_SUPPORT bit is not set\n", mmc_hostname(card->host));
		}
/*cache flush barrier*/
		if (ext_csd[EXT_CSD_BARRIER_SUPPORT]) {
			card->ext_csd.cache_flush_barrier = ext_csd[EXT_CSD_BARRIER_SUPPORT];
			if (card->ext_csd.cache_flush_barrier)
				pr_info("%s: support BARRIER_SUPPORT.\n", mmc_hostname(card->host));
		}
/*cache flush policy*/
		if (ext_csd[EXT_CSD_CACHE_FLUSH_POLICY]) {
			card->ext_csd.cache_flush_policy = ext_csd[EXT_CSD_CACHE_FLUSH_POLICY] & EXT_CSD_CACHE_FLUSH_POLICY_FIFO;
			if (card->ext_csd.cache_flush_policy)
				pr_info("%s: support CACHE_FLUSH_POLICY_FIFO.\n", mmc_hostname(card->host));
		}
/*bkops auto*/
		if (ext_csd[EXT_CSD_BKOPS_SUPPORT] & 0x1) {
			card->ext_csd.bkops_auto_en = ext_csd[EXT_CSD_BKOPS_EN] & EXT_CSD_BKOPS_AUTO_EN;
			if (!card->ext_csd.bkops_auto_en) {
				pr_info("%s: BKOPS_AUTO_EN bit is not set.\n", mmc_hostname(card->host));
			}
		}
/*rpmb 8k*/
		if (ext_csd[EXT_CSD_WR_REL_PARAM] & EXT_CSD_RPMB_REL_WR_EN) {
			pr_info("%s: support RPMB 8K Bytes read/write.\n", mmc_hostname(card->host));
		}
	}

	pr_err("%s: EXT_CSD revision 0x%02x pre_eol_info = 0x%02X device_life_time_est_typ_a = 0x%02X device_life_time_est_typ_b = 0x%02X.\n",
		mmc_hostname(card->host),card->ext_csd.rev, card->ext_csd.pre_eol_info,
		    card->ext_csd.device_life_time_est_typ_a, card->ext_csd.device_life_time_est_typ_b);
}


extern int mmc_screen_test_cache_enable(struct mmc_card *card);
/*enable feature for emmc5.0 or later contains cmdq,barrier and bkops*/
int mmc_init_card_enable_feature(struct mmc_card *card)
{
	int err = 0;
	struct mmc_host *host = card->host;
    if(!mmc_screen_test_cache_enable(card))
    {
        pr_err("%s:Do emmc screen test, Disable all cache related feature, then return \n ",__func__);
        return err;
    }
	/*
	 * Enable command queue (if supported)
	 */
	if (card->ext_csd.cmdq_support) {
		if (host->caps2 & MMC_CAP2_CMD_QUEUE) {
			card->ext_csd.cmdq_mode_en = 1;
			pr_err("%s: cmdq_mode_en=%d\n", __func__, card->ext_csd.cmdq_mode_en);
		} else {
			card->ext_csd.cmdq_mode_en = 0;
		}
	}

	/*
	 * Enable Barrier feature (if supported)
	 */
	if (card->ext_csd.cache_flush_barrier && (host->caps2 & MMC_CAP2_CACHE_FLUSH_BARRIER)) {
		err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_BARRIER_CTRL, 1,
				card->ext_csd.generic_cmd6_time);
		if (err && err != -EBADMSG)
			return err;
		if (err) {
			pr_warning("%s: Enabling cache flush barrier failed\n",
				   mmc_hostname(card->host));
			err = 0;
			card->ext_csd.cache_flush_barrier_en = 0;
		} else {
			card->ext_csd.cache_flush_barrier_en = 1;
			pr_err("%s: cache_flush_barrier_en=%d\n", __func__, card->ext_csd.cache_flush_barrier_en);
		}
	}

	/*
	 * Enable BKOPS AUTO  feature (if supported)
	 */
	if ((host->caps2 & MMC_CAP2_BKOPS_AUTO_CTRL) && (host->pm_flags & MMC_PM_KEEP_POWER) && card->ext_csd.bkops && (card->ext_csd.rev >= 8)) {
		err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_BKOPS_EN, EXT_CSD_BKOPS_AUTO_EN,
				card->ext_csd.generic_cmd6_time);
		if (err && err != -EBADMSG)
			return err;
		if (err) {
			pr_warning("%s: BKOPS_AUTO_EN failed\n",
				   mmc_hostname(card->host));
			err = 0;
			card->ext_csd.bkops_auto_en = 0;
		} else {
			card->ext_csd.bkops_auto_en = 1;
			pr_err("%s: support BKOPS_AUTO_EN, bkops_auto_en=%d\n", __func__, card->ext_csd.bkops_auto_en);
		}
	}
	return err;
}

/*suspend operation on hisi platform, conclude disable cmdq
 *swich to normal partion and so on
 */
int mmc_suspend_hisi_operation(struct mmc_host *host)
{
	unsigned long timeout = 8000;
	u8 part_config;
	int err = 0;
	if (mmc_card_cmdq(host->card)) {
		/* wait for cmdq req handle done.*/
		while (host->cmdq_ctx.active_reqs) {
			if (timeout == 0) {
				pr_err("%s: wait cmdq complete reqs timeout !\n", __func__);
			    err = -ETIMEDOUT;
				goto err_handle;
			}
			timeout--;
			mdelay(1);
		}

		if (host->cmdq_ops->disable) {
			err = host->cmdq_ops->disable(host, true);
			if (err) {
				pr_err("%s: cmdq disable failed.\n", __func__);
				goto err_handle;
			}
		}

		err = mmc_switch(host->card, EXT_CSD_CMD_SET_NORMAL,
			EXT_CSD_CMDQ_MODE, 0,
			host->card->ext_csd.generic_cmd6_time);

		if (err) {
			pr_err("%s: disable device cmdq failed.active_reqs is %lu, "
				"cmdq status is %d.\n", __func__,
				host->cmdq_ctx.active_reqs, !!mmc_card_cmdq(host->card));
			/*we re-enable the device cmdq feature just in case;
			 *For example:1.rpmb_access disable the device cmdq feature;
			 *2.disable device cmdq feqture failed here in _mmc_suspend;
			 *3.if we don't re-enable device cmdq feature,next normal req
			 *will fail;
			 */
			(void)mmc_switch(host->card, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_CMDQ_MODE, 1,
				host->card->ext_csd.generic_cmd6_time);

			if (host->cmdq_ops->enable) {
				if (host->cmdq_ops->enable(host))
					pr_err("%s %d: cmdq enable failed.\n", __func__, __LINE__);
			}
			goto err_handle;
		}

		mmc_card_clr_cmdq(host->card);
	}

	/*
	 * Ensure eMMC user default partition is enabled,because cmd5 will be
	 * a illegal cmd when device is in rpmb partition.
	 */
	if (host->card->ext_csd.part_config & EXT_CSD_PART_CONFIG_ACC_MASK) {
		pr_err("%s need to swich to default partition, current part_config is %d\n",
			__func__, host->card->ext_csd.part_config);
		part_config = host->card->ext_csd.part_config;
		part_config &= ~EXT_CSD_PART_CONFIG_ACC_MASK;
		err = mmc_switch(host->card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONFIG,
				 part_config,
				 host->card->ext_csd.part_time);
		if (err) {
			pr_err("%s swich to default partition failed, err is %d\n",
				__func__, err);
			goto err_handle;
		}
		host->card->ext_csd.part_config = part_config;
	}
err_handle:
	return err;
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
int hisi_mmc_reset(struct mmc_host *host)
{
	int ret;
	struct mmc_card *card = host->card;

	pr_err("%s enter\n", __func__);

	if (!host->ops->hw_reset)
		return -EOPNOTSUPP;

	mmc_host_clk_hold(host);

	host->ops->hw_reset(host);

	mmc_power_off(host);
	mdelay(200);
	mmc_power_up(host, host->card->ocr);

	mmc_host_clk_release(host);
	ret = host->bus_ops->power_restore(host);
	pr_err("%s exit,ret=%d\n", __func__, ret);
	return ret;
}
#pragma GCC diagnostic pop

/*********************sd ops begin**********************/
static int mmc_do_sd_reset(struct mmc_host *host)
{
	struct mmc_card *card = host->card;

	if (!host->bus_ops->power_restore)
		return -EOPNOTSUPP;

	if (!card)
		return -EINVAL;

	mmc_host_clk_hold(host);

	/* TODO! Here add hw_reset for ip reset */
	if (host->ops->hw_reset)
		host->ops->hw_reset(host);

	/* Only for K930/920 SD slow down clk*/
	if (host->ops->slowdown_clk)
		host->ops->slowdown_clk(host, host->ios.timing);

	mmc_power_off(host);
	mmc_set_clock(host, host->f_init);
	/* Wait at least 200 ms */
	mmc_delay(200);
	mmc_power_up(host,host->card->ocr);
	(void)mmc_select_voltage(host, host->card->ocr);

	mmc_host_clk_release(host);

	return host->bus_ops->power_restore(host);
}

int mmc_sd_reset(struct mmc_host *host)
{
	return mmc_do_sd_reset(host);
}
EXPORT_SYMBOL(mmc_sd_reset);

/*低速卡，设定频率25M  设置卡位宽*/
void mmc_select_new_sd(struct mmc_card *card)
{
	unsigned int max_dtr;

	mmc_set_timing(card->host, MMC_TIMING_NEW_SD);
	max_dtr = (unsigned int)-1;
	if (max_dtr > card->csd.max_dtr) {
		max_dtr = card->csd.max_dtr;
	}
	mmc_set_clock(card->host, max_dtr);
}



/*********************wifi ops begin**********************/
int mmc_power_save_host_for_wifi(struct mmc_host *host)
{
	int ret = 0;

#ifdef CONFIG_MMC_DEBUG
	pr_info("%s: %s: powering down\n", mmc_hostname(host), __func__);
#endif

	mmc_bus_get(host);

	if ( (!host->ops || !host->ops->set_ios) ||
         (host->bus_dead) ) {
		mmc_bus_put(host);
		return -EINVAL;
	}

	mmc_bus_put(host);

	mmc_power_off(host);

	return ret;
}
EXPORT_SYMBOL(mmc_power_save_host_for_wifi);

int mmc_power_restore_host_for_wifi(struct mmc_host *host)
{
	int ret = 0;

#ifdef CONFIG_MMC_DEBUG
	pr_info("%s: %s: powering up\n", mmc_hostname(host), __func__);
#endif

	mmc_bus_get(host);

	if ( (!host->ops || !host->ops->set_ios) ||
         (host->bus_dead) ) {
		mmc_bus_put(host);
		return -EINVAL;
	}

	mmc_power_up(host,host->card->ocr);

	mmc_bus_put(host);

	return ret;
}
EXPORT_SYMBOL(mmc_power_restore_host_for_wifi);




