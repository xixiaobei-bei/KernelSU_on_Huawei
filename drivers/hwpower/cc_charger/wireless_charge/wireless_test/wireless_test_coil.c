/*
 * wireless_test_coil.c
 *
 * wireless test coil
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/wireless_charge/wireless_test_hw.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>

#define HWLOG_TAG wireless_tst_coil
HWLOG_REGIST();

#define WIRELESS_COIL_TEST_SUCC               0
#define WIRELESS_COIL_TEST_FAIL               1
#define WIRELESS_COIL_TEST_RUNNING            2
#define WIRELESS_COIL_TEST_IIN_MIN            100

struct wireless_coil_dev {
	struct work_struct chk_coil_work;
	int test_result;
	bool test_busy;
};

static struct wireless_coil_dev *g_wireless_coil_di;

/*
 * function: calculate tx_avg_iin
 * tx_avg_iin = iin_sum / cnt, 10 <= cnt <= 20
 * if tx_iin > 100mA, iin_sum += tx_iin, otherwise cnt--
 * 20: calculate average tx_iin maximum count
 * 10: calculate average tx_iin minimal count
 * 100: tx_iin low threshold
 *
 */
static int wireless_coil_test_get_avg_iin(int *tx_avg_iin)
{
	int i;
	u16 tx_iin = 0;
	int iin_sum = 0;
	int cnt = 20;

	for (i = 0; i < 20; i++) {
		power_usleep(DT_USLEEP_5MS);
		if (wltx_ic_get_iin(WLTRX_IC_MAIN, &tx_iin)) {
			hwlog_err("get_tx_iin: failed\n");
			return -EINVAL;
		}
		if ((tx_iin < WIRELESS_COIL_TEST_IIN_MIN) && (cnt > 10))
			cnt--;
		else
			iin_sum += tx_iin;
		hwlog_info("[get_avg_iin] tx_iin=%umA\n", tx_iin);
	}

	*tx_avg_iin = iin_sum / cnt;
	hwlog_info("[get_avg_iin] tx_avg_iin=%dmA,cnt=%d\n", *tx_avg_iin, cnt);
	return 0;
}

static int wireless_coil_contact_test(void)
{
	int ret;
	int tx_avg_iin = 0;

	ret = wireless_coil_test_get_avg_iin(&tx_avg_iin);
	if (ret || (tx_avg_iin < WIRELESS_COIL_TEST_IIN_MIN)) {
		hwlog_err("contact_test: failed\n");
		ret = -EINVAL;
		goto close_tx;
	}

	hwlog_info("[contact_test] succ\n");

close_tx:
	wltx_open_tx(WLTX_OPEN_BY_COIL_TEST, false);
	return ret;
}

static void wireless_coil_test_work(struct work_struct *work)
{
	struct wireless_coil_dev *di = container_of(work, struct wireless_coil_dev, chk_coil_work);

	if (!di)
		return;

	if (wireless_coil_contact_test())
		di->test_result = WIRELESS_COIL_TEST_FAIL;
	else
		di->test_result = WIRELESS_COIL_TEST_SUCC;

	di->test_busy = false;
}

static bool wireless_coil_test_open_tx(void)
{
	int i;

	wltx_open_tx(WLTX_OPEN_BY_COIL_TEST, true);

	/* wait for 100ms*50=5000ms for ping ready */
	for (i = 0; i < 50; i++) {
		power_msleep(DT_MSLEEP_100MS, 0, NULL);
		if (wireless_tx_get_stage() >= WL_TX_STAGE_PING_RX)
			return true;
	}

	hwlog_err("open_tx: wait ping timeout\n");
	return false;
}

void wireless_coil_test_start(void)
{
	struct wireless_coil_dev *di = g_wireless_coil_di;

	if (!di)
		return;

	if (di->test_busy) {
		hwlog_err("test_start: test busy\n");
		di->test_result = WIRELESS_COIL_TEST_RUNNING;
		return;
	}

	di->test_busy = true;

	if (wireless_coil_test_open_tx()) {
		schedule_work(&di->chk_coil_work);
		return;
	}

	di->test_result = WIRELESS_COIL_TEST_FAIL;
	di->test_busy = false;
}

int wirless_coil_test_result(void)
{
	struct wireless_coil_dev *di = g_wireless_coil_di;

	if (!di)
		return -ENOMEM;

	return di->test_result;
}

static int __init wireless_coil_test_init(void)
{
	struct wireless_coil_dev *di = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_wireless_coil_di = di;
	INIT_WORK(&di->chk_coil_work, wireless_coil_test_work);

	hwlog_info("[module_init] succ\n");
	return 0;
}

static void __exit wireless_coil_test_exit(void)
{
	struct wireless_coil_dev *di = g_wireless_coil_di;

	if (!di)
		return;

	kfree(di);
	g_wireless_coil_di = NULL;
}

module_init(wireless_coil_test_init);
module_exit(wireless_coil_test_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless coil test module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
