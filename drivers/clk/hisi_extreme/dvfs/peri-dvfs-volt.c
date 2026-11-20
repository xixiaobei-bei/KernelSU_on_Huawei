/*
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/sched.h>
#include "peri-volt-internal.h"

static struct task_struct *perivolt_owner;
static int g_perivolt_refcnt;
static LIST_HEAD(perivolt_list);
static DEFINE_MUTEX(perivolt_poll_lock);

/* locking */
static void perivolt_lock(void)
{
	if (!mutex_trylock(&perivolt_poll_lock)) {
		if (perivolt_owner == current) {
			g_perivolt_refcnt++;
			return;
		}
		mutex_lock(&perivolt_poll_lock);
	}
	WARN_ON_ONCE(perivolt_owner != NULL);
	WARN_ON_ONCE(g_perivolt_refcnt != 0);
	perivolt_owner = current;
	g_perivolt_refcnt = 1;
}

static void perivolt_unlock(void)
{
	WARN_ON_ONCE(perivolt_owner != current);
	WARN_ON_ONCE(g_perivolt_refcnt == 0);

	if (--g_perivolt_refcnt)
		return;
	perivolt_owner = NULL;
	mutex_unlock(&perivolt_poll_lock);
}

static unsigned int __peri_get_volt(struct peri_volt_poll *pvp)
{
	return pvp->volt;
}

static struct peri_volt_poll *__perivolt_lookup(unsigned int dev_id, const char *name)
{
	struct peri_volt_poll *pvp = NULL;

	if (name != NULL) {
		list_for_each_entry(pvp, &perivolt_list, node)
			if (!strncmp(pvp->name, name,
				strlen(name) > strlen(pvp->name) ? strlen(name) : strlen(pvp->name)))
				return pvp;
	}
	if (dev_id) {
		list_for_each_entry(pvp, &perivolt_list, node)
			if (dev_id == pvp->dev_id)
				return pvp;
	}

	return NULL;
}

static unsigned int __peri_recalc_volt(struct peri_volt_poll *pvp)
{
	if (pvp->ops->recalc_volt)
		pvp->volt = pvp->ops->recalc_volt(pvp);

	return pvp->volt;
}

struct peri_volt_poll *peri_volt_poll_get(unsigned int dev_id, const char *name)
{
	struct peri_volt_poll *pvp = NULL;

	if ((!dev_id) && (name == NULL))
		return NULL;

	perivolt_lock();
	pvp = __perivolt_lookup(dev_id, name);
	perivolt_unlock();

	return pvp;
}

/*
 * peri_get_volt - return the volt of peri dvfs
 * @pvp: the peri_volt_poll whose volt is being returned.
 */
unsigned int peri_get_volt(struct peri_volt_poll *pvp)
{
	unsigned int volt;

	if (pvp == NULL)
		return (unsigned int)(-EINVAL);

	perivolt_lock();

	volt = __peri_get_volt(pvp);

	if (pvp && (pvp->flags & PERI_GET_VOLT_NOCACHE))
		volt = __peri_recalc_volt(pvp);

	perivolt_unlock();

	return volt;
}

static int __peri_set_volt(struct peri_volt_poll *pvp, unsigned int volt)
{
	int ret = 0;

	if (pvp->ops->set_volt)
		ret = pvp->ops->set_volt(pvp, volt);

	return ret;
}

/*
 * peri_set_volt - specify a new volt for pvp
 * @pvp: the peri_volt_poll whose volt is being changed
 * @volt: the new rate for pvp
 *
 * Returns 0 on success, -EERROR otherwise.
 */
int peri_set_volt(struct peri_volt_poll *pvp, unsigned int volt)
{
	int ret;

	if (pvp == NULL)
		return -EINVAL;

	/* prevent racing with updates to the clock topology */
	perivolt_lock();

	/* change the rates */
	ret = __peri_set_volt(pvp, volt);
	if (ret == -EPERM)
		pr_err("[%s] %s peri dvfs is off\n", __func__, pvp->name);
	pvp->volt = volt;

	perivolt_unlock();

	return ret;
}

static int __peri_get_temperature(struct peri_volt_poll *pvp)
{
	int ret = 0;

	if (pvp->ops->get_temperature)
		ret = pvp->ops->get_temperature(pvp);

	return ret;
}

/*
 * peri_get_temprature - get current temperature
 * Returns 0 on normal temperature, -1 on low temperature.
 */
int peri_get_temperature(struct peri_volt_poll *pvp)
{
	int ret;

	if (pvp == NULL)
		return -EINVAL;

	/* prevent racing with updates to the clock topology */
	perivolt_lock();

	/* change the rates */
	ret = __peri_get_temperature(pvp);
	perivolt_unlock();

	return ret;
}

/*
 * peri_set_avs - set avs poll for pvp
 * @pvp: the peri_volt_poll whose avs poll will changed
 * @avs: the new avs poll set for pvp
 *
 * Returns 0 on success, -EERROR otherwise.
 */
int peri_set_avs(struct peri_volt_poll *pvp, unsigned int avs)
{
	int ret = 0;

	if (IS_ERR_OR_NULL(pvp))
		return -EINVAL;

	perivolt_lock();
	if (pvp->ops->set_avs)
		ret = pvp->ops->set_avs(pvp, avs);
	perivolt_unlock();

	return ret;
}

/*
 * peri_wait_avs_update - wait avs status updated
 * @pvp: the peri_volt_poll whose avs poll will changed
 *
 * Returns 0 on success, -EERROR otherwise.
 */
int peri_wait_avs_update(struct peri_volt_poll *pvp)
{
	int ret = 0;

	if (IS_ERR_OR_NULL(pvp))
		return -EINVAL;

	perivolt_lock();
	if (pvp->ops->wait_avs_update)
		ret = pvp->ops->wait_avs_update(pvp);
	perivolt_unlock();

	return ret;
}

int __perivolt_init(struct device *dev, struct peri_volt_poll *pvp)
{
	int ret = 0;

	if (pvp == NULL)
		return -EINVAL;

	perivolt_lock();

	/* check to see if a pvp with this name is already registered */
	if (__perivolt_lookup(0, pvp->name)) {
		pr_debug("%s: pvp %u already initialized\n", __func__, pvp->dev_id);
		ret = -EEXIST;
		goto out;
	}
	list_add(&pvp->node, &perivolt_list);

	if (pvp->ops->init)
		pvp->ops->init(pvp, 0);

out:
	perivolt_unlock();

	return ret;
}

/*
 * perivolt_register - register a peri_volt_poll and return a cookie.
 * Returns 0 on success, otherwise an error code.
 */
int perivolt_register(struct device *dev, struct peri_volt_poll *pvp)
{
	struct peri_volt_poll *perivolt = NULL;
	int ret;

	if (pvp == NULL)
		return -EINVAL;

	perivolt = kzalloc(sizeof(*pvp), GFP_KERNEL);
	if (perivolt == NULL)
		return -ENOMEM;

	perivolt->name = kstrdup(pvp->name, GFP_KERNEL);
	if (perivolt->name == NULL) {
		ret = -ENOMEM;
		goto fail_pvp;
	}
	perivolt->dev_id = pvp->dev_id;
	perivolt->perivolt_avs_ip = pvp->perivolt_avs_ip;
	perivolt->addr = pvp->addr;
	perivolt->addr_0 = pvp->addr_0;
	perivolt->sysreg_base = pvp->sysreg_base;
	perivolt->bitsmask = pvp->bitsmask;
	perivolt->bitsshift = pvp->bitsshift;
	perivolt->ops = pvp->ops;
	perivolt->volt = pvp->volt;
	perivolt->flags = pvp->flags;
	perivolt->priv = pvp->priv;
	perivolt->stat = pvp->stat;
#ifdef CONFIG_HISI_HW_PERI_DVS
	perivolt->priv_date = pvp->priv_date;
#endif
	pr_info("%s: id=%u name=%s\n", __func__, perivolt->dev_id,
		perivolt->name);
	ret = __perivolt_init(dev, perivolt);
	if (!ret)
		return ret;

	kfree(perivolt->name);

fail_pvp:
	kfree(perivolt);
	return ret;
}

