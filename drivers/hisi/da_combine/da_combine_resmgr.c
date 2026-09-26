/*
 * da_combine_resmgr.c
 *
 * da_combine_resmgr codec driver
 *
 * Copyright (c) 2014-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/hisi/da_combine/da_combine_resmgr.h>

#include <linux/module.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/notifier.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/hisi/audio_log.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif

#define LOG_TAG "DA_combine_resmgr"

enum pll_rel_state {
	REL_IDLE,
	REL_WAIT,
	REL_PENDING
};

#define PLL_LOCK_MAX_RETRY 5

struct da_combine_resmgr_priv {
	struct da_combine_resmgr resmgr;

	struct resmgr_config config;

	struct mutex pll_mutex;
	int pll_ref_count[PLL_MAX];
	enum hi_cdcctrl_clk pll_source[PLL_MAX];
	enum da_combine_pll_type curr_pll;
	enum da_combine_pll_type reg_acc_pll;
	int reg_acc_ref_count;

	struct mutex micbias_mutex;
	int micbias_ref_count;
	struct mutex ibias_mutex;
	int ibias_ref_count;
	struct mutex supply_mutex;
	int supply_ref_count;

	wait_queue_head_t pll_rel_wq;
	enum pll_rel_state pll_wait_rel[PLL_MAX];
	struct task_struct *pll_rel_thrd;
	struct wakeup_source wake_lock;

	struct blocking_notifier_head notifier;

	struct snd_soc_component *codec;
	struct hi_cdc_ctrl *cdc_ctrl;
	struct da_combine_irq *irqmgr;
};

static void dump_pll_state(const struct da_combine_resmgr_priv *priv)
{
	int i;

	AUDIO_LOGI("pll state dump begin");

	AUDIO_LOGI("current pll state: %d", priv->curr_pll);
	for (i = 0; i < priv->config.pll_num; i++)
		AUDIO_LOGI("pll[%d]: ref cnt - %d, wait ref state - %d", i,
			priv->pll_ref_count[i], priv->pll_wait_rel[i]);

	AUDIO_LOGI("register accessing pll: %d, register accessing ref cnt: %d",
		priv->reg_acc_pll, priv->reg_acc_ref_count);

	AUDIO_LOGI("pll state dump end");
}

static void event_notify(struct da_combine_resmgr_priv *priv,
	enum da_combine_resmgr_event event, void *payload)
{
	unsigned long val = event;

	blocking_notifier_call_chain(&priv->notifier, val, payload);
}

static void request_ibias(struct da_combine_resmgr_priv *priv)
{
	mutex_lock(&priv->ibias_mutex);
	if (++priv->ibias_ref_count == 1) {
		hi_cdcctrl_enable_supply(priv->cdc_ctrl, CDC_SUP_ANLG, true);
		priv->config.enable_ibias(priv->codec);
	}

	mutex_unlock(&priv->ibias_mutex);
}

static void release_ibias(struct da_combine_resmgr_priv *priv)
{
	mutex_lock(&priv->ibias_mutex);
	if (priv->ibias_ref_count == 0) {
		AUDIO_LOGE("release ibias when reference counter is 0");
		mutex_unlock(&priv->ibias_mutex);
		return;
	}

	if (--priv->ibias_ref_count == 0) {
		priv->config.disable_ibias(priv->codec);
		hi_cdcctrl_enable_supply(priv->cdc_ctrl, CDC_SUP_ANLG, false);
	}

	mutex_unlock(&priv->ibias_mutex);
}

static void request_supply(struct da_combine_resmgr_priv *priv)
{
	mutex_lock(&priv->supply_mutex);
	if (priv->config.enable_supply != NULL && ++priv->supply_ref_count == 1)
		priv->config.enable_supply(priv->codec);

	mutex_unlock(&priv->supply_mutex);
}

static void release_supply(struct da_combine_resmgr_priv *priv)
{
	mutex_lock(&priv->supply_mutex);
	if (priv->supply_ref_count == 0) {
		AUDIO_LOGE("release supply when reference counter is 0");
		mutex_unlock(&priv->supply_mutex);
		return;
	}

	if (priv->config.disable_supply != NULL && --priv->supply_ref_count == 0)
		priv->config.disable_supply(priv->codec);

	mutex_unlock(&priv->supply_mutex);
}

static void request_micbias(struct da_combine_resmgr_priv *priv)
{
	mutex_lock(&priv->micbias_mutex);
	if (++priv->micbias_ref_count == 1) {
		request_ibias(priv);
		priv->config.enable_micbias(priv->codec);
	}

	mutex_unlock(&priv->micbias_mutex);
}

static int release_micbias(struct da_combine_resmgr_priv *priv, bool force_release)
{
	mutex_lock(&priv->micbias_mutex);

	if (priv->micbias_ref_count == 0) {
		AUDIO_LOGW("release micbias when reference counter is 0");
		mutex_unlock(&priv->micbias_mutex);
		return -EINVAL;
	}

	if (force_release)
		priv->micbias_ref_count = 1;

	if (--priv->micbias_ref_count == 0) {
		priv->config.disable_micbias(priv->codec);
		release_ibias(priv);
	}

	mutex_unlock(&priv->micbias_mutex);

	return 0;
}

static int turn_on_pll(const struct da_combine_resmgr_priv *priv,
	enum da_combine_pll_type pll_type)
{
	hi_cdcctrl_enable_clk(priv->cdc_ctrl, priv->pll_source[pll_type], true);

	return priv->config.pfn_pll_ctrls[pll_type].turn_on(priv->codec);
}

static void turn_off_pll(const struct da_combine_resmgr_priv *priv,
	enum da_combine_pll_type pll_type)
{
	priv->config.pfn_pll_ctrls[pll_type].turn_off(priv->codec);

	hi_cdcctrl_enable_clk(priv->cdc_ctrl, priv->pll_source[pll_type], false);
}

static void lock_pll(const struct da_combine_resmgr_priv *priv, enum da_combine_pll_type pll_type)
{
	bool pll_locked = false;
	int retry = PLL_LOCK_MAX_RETRY;

	do {
		if (turn_on_pll(priv, pll_type)) {
			AUDIO_LOGW("try to turn on pll[%d] failed", pll_type);
			usleep_range(1000, 1100);
			continue;
		}

		pll_locked = true;
		/* check pll lock state for some times */
		if (!priv->config.pfn_pll_ctrls[pll_type].is_locked(priv->codec)) {
			AUDIO_LOGW("pll[%d] is not locked after turn on, retry time is %d",
				pll_type, (PLL_LOCK_MAX_RETRY - retry));

			pll_locked = false;
		}

		if (pll_locked)
			break;

		turn_off_pll(priv, pll_type);
		usleep_range(5000, 5100);
	} while (--retry);

	if (!pll_locked) {
		AUDIO_LOGE("failed to lock pll[%d]", pll_type);
#ifdef CONFIG_HUAWEI_DSM
		audio_dsm_report_info(AUDIO_CODEC, DSM_HI6402_PLL_CANNOT_LOCK,
			"da combine codec failed to lock pll after retry\n");
#endif
		/* Anyway, turn on the pll to provide clock */
		(void)turn_on_pll(priv, pll_type);
	}
}

static int request_pll_single(struct da_combine_resmgr_priv *priv,
	enum da_combine_pll_type pll_type)
{
	struct pll_switch_event event;

	if (pll_type < 0 || pll_type >= priv->config.pll_num) {
		AUDIO_LOGE("pll type error, %d", pll_type);
		WARN_ON(1);
		return -EINVAL;
	}

	mutex_lock(&priv->pll_mutex);

	if (++priv->pll_ref_count[pll_type] == 1 && pll_type > priv->curr_pll) {
		event.from = priv->curr_pll;
		event.to = pll_type;
		event_notify(priv, PRE_PLL_SWITCH, &event);

		if (priv->curr_pll == PLL_NONE) {
			request_supply(priv);
			request_ibias(priv);
		} else {
			turn_off_pll(priv, priv->curr_pll);
		}

		lock_pll(priv, pll_type);
		priv->curr_pll = pll_type;

		event_notify(priv, POST_PLL_SWITCH, &event);
	}

	mutex_unlock(&priv->pll_mutex);

	return 0;
}

static int request_pll_multiple(struct da_combine_resmgr_priv *priv,
	enum da_combine_pll_type pll_type)
{
	struct pll_switch_event event;

	if (pll_type < 0 || pll_type >= priv->config.pll_num) {
		AUDIO_LOGE("pll type error, %d", pll_type);
		WARN_ON(1);
		return -EINVAL;
	}

	mutex_lock(&priv->pll_mutex);

	if (pll_type != PLL_44_1) {
		if (++priv->pll_ref_count[pll_type] == 1 && pll_type > priv->curr_pll) {
			event.from = priv->curr_pll;
			event.to = pll_type;

			if (priv->curr_pll == PLL_NONE) {
				request_supply(priv);
				request_ibias(priv);
				lock_pll(priv, pll_type);
				event_notify(priv, PRE_PLL_SWITCH, &event);
				event_notify(priv, POST_PLL_SWITCH, &event);
			} else {
				lock_pll(priv, pll_type);
				event_notify(priv, PRE_PLL_SWITCH, &event);
				event_notify(priv, POST_PLL_SWITCH, &event);
				turn_off_pll(priv, priv->curr_pll);
			}

			priv->curr_pll = pll_type;
		}
	} else {
		if (++priv->pll_ref_count[pll_type] == 1) {
			request_supply(priv);
			request_ibias(priv);
			lock_pll(priv, pll_type);
		}
	}

	mutex_unlock(&priv->pll_mutex);

	return 0;
}

static int release_pll_single(struct da_combine_resmgr_priv *priv,
	enum da_combine_pll_type pll_type)
{
	int i;
	struct pll_switch_event event;
	enum da_combine_pll_type new_pll;

	if (pll_type < 0 || pll_type >= priv->config.pll_num) {
		AUDIO_LOGE("pll type error, %d", pll_type);
		WARN_ON(1);
		return -EINVAL;
	}

	mutex_lock(&priv->pll_mutex);

	if (priv->pll_ref_count[pll_type] == 0) {
		AUDIO_LOGE("release pll[%d] when reference counter is 0", pll_type);
		mutex_unlock(&priv->pll_mutex);
		return -EINVAL;
	}

	if (--priv->pll_ref_count[pll_type] == 0 && pll_type == priv->curr_pll) {
		new_pll = PLL_NONE;
		for (i = pll_type - 1; i >= 0; i--) {
			if (priv->pll_ref_count[i] > 0) {
				new_pll = (enum da_combine_pll_type)i;
				break;
			}
		}

		event.from = pll_type;
		event.to = new_pll;
		event_notify(priv, PRE_PLL_SWITCH, &event);

		turn_off_pll(priv, pll_type);

		if (new_pll == PLL_NONE) {
			release_ibias(priv);
			release_supply(priv);
		} else {
			lock_pll(priv, new_pll);
		}

		priv->curr_pll = new_pll;

		event_notify(priv, POST_PLL_SWITCH, &event);
	}

	mutex_unlock(&priv->pll_mutex);

	return 0;
}

static int release_pll_multiple(struct da_combine_resmgr_priv *priv,
	enum da_combine_pll_type pll_type)
{
	int i;
	enum da_combine_pll_type new_pll;
	struct pll_switch_event event;

	if (pll_type < 0 || pll_type >= priv->config.pll_num) {
		AUDIO_LOGE("pll type error, %d", pll_type);
		return -EINVAL;
	}

	mutex_lock(&priv->pll_mutex);

	if (priv->pll_ref_count[pll_type] == 0) {
		AUDIO_LOGE("release pll[%d] when reference counter is 0", pll_type);
		mutex_unlock(&priv->pll_mutex);
		return -EINVAL;
	}

	if (pll_type == PLL_44_1) {
		if (--priv->pll_ref_count[pll_type] == 0) {
			turn_off_pll(priv, pll_type);
			release_ibias(priv);
			release_supply(priv);
		}

		mutex_unlock(&priv->pll_mutex);

		return 0;
	}

	if (--priv->pll_ref_count[pll_type] == 0 && pll_type == priv->curr_pll) {
		new_pll = PLL_NONE;
		for (i = pll_type - 1; i >= 0; i--) {
			if (priv->pll_ref_count[i] > 0) {
				new_pll = (enum da_combine_pll_type)i;
				break;
			}
		}

		event.from = pll_type;
		event.to = new_pll;

		if (new_pll == PLL_NONE) {
			event_notify(priv, PRE_PLL_SWITCH, &event);
			event_notify(priv, POST_PLL_SWITCH, &event);
			turn_off_pll(priv, pll_type);
			release_ibias(priv);
			release_supply(priv);
		} else {
			lock_pll(priv, new_pll);
			event_notify(priv, PRE_PLL_SWITCH, &event);
			event_notify(priv, POST_PLL_SWITCH, &event);
			turn_off_pll(priv, pll_type);
		}

		priv->curr_pll = new_pll;
	}

	mutex_unlock(&priv->pll_mutex);

	return 0;
}

static bool is_pll_waiting_release(const struct da_combine_resmgr_priv *priv)
{
	int i;

	for (i = 0; i < PLL_MAX; i++) {
		if (priv->pll_wait_rel[i] != REL_IDLE)
			return true;
	}

	return false;
}

static void pll_delay_release(struct da_combine_resmgr_priv *priv,
	enum da_combine_pll_type pll_type)
{
	spin_lock(&priv->pll_rel_wq.lock);

	if (priv->pll_wait_rel[pll_type] == REL_IDLE) {
		priv->pll_wait_rel[pll_type] = REL_WAIT;
		wake_up_locked(&priv->pll_rel_wq);
		spin_unlock(&priv->pll_rel_wq.lock);
	} else {
		if (priv->pll_wait_rel[pll_type] == REL_PENDING)
			priv->pll_wait_rel[pll_type] = REL_WAIT;
		spin_unlock(&priv->pll_rel_wq.lock);

		if (priv->config.pll_sw_mode == MODE_SINGLE)
			(void)release_pll_single(priv, pll_type);
		else
			(void)release_pll_multiple(priv, pll_type);
	}
}

static void release_pll_by_state(struct da_combine_resmgr_priv *priv, bool *pending)
{
	int i;

	for (i = 0; i < PLL_MAX; i++) {
		spin_lock(&priv->pll_rel_wq.lock);
		if (priv->pll_wait_rel[i] == REL_PENDING) {
			priv->pll_wait_rel[i] = REL_IDLE;
			spin_unlock(&priv->pll_rel_wq.lock);
			if (priv->config.pll_sw_mode == MODE_SINGLE)
				release_pll_single(priv, (enum da_combine_pll_type)i);
			else
				release_pll_multiple(priv, (enum da_combine_pll_type)i);
		} else if (priv->pll_wait_rel[i] == REL_WAIT) {
			priv->pll_wait_rel[i] = REL_PENDING;
			*pending = true;
			spin_unlock(&priv->pll_rel_wq.lock);
		} else {
			spin_unlock(&priv->pll_rel_wq.lock);
		}
	}
}

static int pll_delayed_release_thread(void *data)
{
	int i;
	bool pending = false;
	struct da_combine_resmgr_priv *priv = data;

	while (!kthread_should_stop()) {
		spin_lock(&priv->pll_rel_wq.lock);

		wait_event_interruptible_locked(priv->pll_rel_wq,
			(is_pll_waiting_release(priv) || kthread_should_stop()));

		for (i = 0; i < PLL_MAX; i++) {
			if (priv->pll_wait_rel[i] == REL_WAIT) {
				priv->pll_wait_rel[i] = REL_PENDING;
				pending = true;
			}
		}
		spin_unlock(&priv->pll_rel_wq.lock);

		__pm_stay_awake(&priv->wake_lock);
		while (pending && !kthread_should_stop()) {
			msleep(200);
			pending = false;

			release_pll_by_state(priv, &pending);
		}
		__pm_relax(&priv->wake_lock);
	}

	return 0;
}

int da_combine_resmgr_init(struct snd_soc_component *codec, struct hi_cdc_ctrl *cdc_ctrl,
	struct da_combine_irq *irqmgr, const struct resmgr_config *config,
	struct da_combine_resmgr **resmgr)
{
	int ret;

	struct da_combine_resmgr_priv *priv = kzalloc(sizeof(*priv), GFP_KERNEL);

	if (priv == NULL) {
		AUDIO_LOGE("resmgr priv is null");
		return -ENOMEM;
	}

	memcpy(&priv->config, config, sizeof(*config));
	priv->pll_source[PLL_LOW] = CDC_32K;
	priv->pll_source[PLL_HIGH] = CDC_MCLK;
	priv->pll_source[PLL_44_1] = CDC_MCLK;
	priv->curr_pll = PLL_NONE;
	priv->reg_acc_pll = PLL_NONE;
	priv->codec = codec;
	priv->irqmgr = irqmgr;
	priv->cdc_ctrl = cdc_ctrl;

	mutex_init(&priv->pll_mutex);
	mutex_init(&priv->micbias_mutex);
	mutex_init(&priv->ibias_mutex);
	mutex_init(&priv->supply_mutex);

	wakeup_source_init(&priv->wake_lock, "da_combine-resmgr");

	BLOCKING_INIT_NOTIFIER_HEAD(&priv->notifier);

	init_waitqueue_head(&priv->pll_rel_wq);

	priv->pll_rel_thrd = kthread_run(pll_delayed_release_thread,
		priv, "pll_delayed_release");
	if (IS_ERR(priv->pll_rel_thrd)) {
		AUDIO_LOGE("fail to create thread");
		ret = -EIO;
		goto error_exit;
	}

	*resmgr = &priv->resmgr;

	return 0;

error_exit:
	mutex_destroy(&priv->pll_mutex);
	mutex_destroy(&priv->micbias_mutex);
	mutex_destroy(&priv->ibias_mutex);
	mutex_destroy(&priv->supply_mutex);
	wakeup_source_trash(&priv->wake_lock);

	kfree(priv);
	priv = NULL;

	return ret;
}

void da_combine_resmgr_deinit(struct da_combine_resmgr *resmgr)
{
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL)
		return;

	kthread_stop(priv->pll_rel_thrd);
	priv->pll_rel_thrd = NULL;

	mutex_destroy(&priv->pll_mutex);
	mutex_destroy(&priv->micbias_mutex);
	mutex_destroy(&priv->ibias_mutex);
	mutex_destroy(&priv->supply_mutex);
	wakeup_source_trash(&priv->wake_lock);

	kfree(priv);
	priv = NULL;
}

static enum da_combine_pll_type get_reg_acc_pll(struct da_combine_resmgr_priv *priv,
	enum da_combine_pll_type pll)
{
	enum da_combine_pll_type ret = pll;

	mutex_lock(&priv->pll_mutex);

	if (priv->reg_acc_pll != PLL_NONE) {
		ret = priv->reg_acc_pll;
	} else {
		if (priv->curr_pll != PLL_NONE)
			ret = priv->curr_pll;
	}

	++priv->reg_acc_ref_count;
	priv->reg_acc_pll = ret;

	mutex_unlock(&priv->pll_mutex);

	return ret;
}

static enum da_combine_pll_type put_reg_acc_pll(struct da_combine_resmgr_priv *priv)
{
	enum da_combine_pll_type ret = PLL_NONE;

	mutex_lock(&priv->pll_mutex);

	if (priv->reg_acc_pll == PLL_NONE) {
		AUDIO_LOGE("pll to release for register accessing should not be NONE");
		goto exit;
	}

	if (priv->reg_acc_ref_count == 0) {
		AUDIO_LOGE("ref count for register accessing is wrong");
		goto exit;
	}

	ret = priv->reg_acc_pll;
	if (--priv->reg_acc_ref_count == 0)
		priv->reg_acc_pll = PLL_NONE;

exit:
	mutex_unlock(&priv->pll_mutex);

	return ret;
}

int da_combine_resmgr_request_reg_access(struct da_combine_resmgr *resmgr,
	unsigned int reg_addr)
{
	int ret;
	enum da_combine_pll_type req_pll;
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return -EINVAL;
	}

	req_pll = priv->config.pll_for_reg_access(priv->codec, reg_addr);
	if (req_pll == PLL_NONE)
		return 0;

	req_pll = get_reg_acc_pll(priv, req_pll);

	if (priv->config.pll_sw_mode == MODE_SINGLE)
		ret = request_pll_single(priv, req_pll);
	else
		ret = request_pll_multiple(priv, req_pll);

	/* don't allow pll switch during register access */
	mutex_lock(&priv->pll_mutex);

	return ret;
}

void da_combine_resmgr_release_reg_access(struct da_combine_resmgr *resmgr,
	unsigned int reg_addr)
{
	enum da_combine_pll_type req_pll;
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return;
	}

	req_pll = priv->config.pll_for_reg_access(priv->codec, reg_addr);
	if (req_pll == PLL_NONE)
		return;

	/* release the lock aquired in da_combine_resmgr_request_reg_access */
	mutex_unlock(&priv->pll_mutex);

	req_pll = put_reg_acc_pll(priv);
	if (req_pll == PLL_NONE)
		return;

	pll_delay_release(priv, req_pll);
}

int da_combine_resmgr_request_pll(struct da_combine_resmgr *resmgr,
	enum da_combine_pll_type pll_type)
{
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return 0;
	}

	if (priv->config.pll_sw_mode == MODE_SINGLE)
		return request_pll_single(priv, pll_type);
	else
		return request_pll_multiple(priv, pll_type);
}

int da_combine_resmgr_release_pll(struct da_combine_resmgr *resmgr,
	enum da_combine_pll_type pll_type)
{
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return 0;
	}

	if (priv->config.pll_sw_mode == MODE_SINGLE)
		return release_pll_single(priv, pll_type);
	else
		return release_pll_multiple(priv, pll_type);
}

void da_combine_resmgr_request_micbias(struct da_combine_resmgr *resmgr)
{
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return;
	}

	request_micbias(priv);
}

int da_combine_resmgr_release_micbias(struct da_combine_resmgr *resmgr)
{
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return 0;
	}

	return release_micbias(priv, false);
}

int da_combine_resmgr_force_release_micbias(struct da_combine_resmgr *resmgr)
{
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return 0;
	}

	return release_micbias(priv, true);
}

void da_combine_resmgr_hs_high_resistence_enable(struct da_combine_resmgr *resmgr, bool enable)
{
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	if (priv->config.da_combine_hs_high_resistance_enable != NULL)
		priv->config.da_combine_hs_high_resistance_enable(priv->codec, enable);
}

void da_combine_resmgr_pm_get_clk(void)
{
	hi_cdcctrl_pm_get();
}

void da_combine_resmgr_pm_put_clk(void)
{
	hi_cdcctrl_pm_put();
}

int da_combine_resmgr_register_notifier(struct da_combine_resmgr *resmgr,
	struct notifier_block *nblock)
{
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return 0;
	}

	return blocking_notifier_chain_register(&priv->notifier, nblock);
}

int da_combine_resmgr_unregister_notifier(struct da_combine_resmgr *resmgr,
	struct notifier_block *nblock)
{
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return 0;
	}

	return blocking_notifier_chain_unregister(&priv->notifier, nblock);
}

void da_combine_resmgr_dump(struct da_combine_resmgr *resmgr)
{
	struct da_combine_resmgr_priv *priv = (struct da_combine_resmgr_priv *)resmgr;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return;
	}

	dump_pll_state(priv);
}

