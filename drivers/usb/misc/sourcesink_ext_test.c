/*
 * sourcesink_ext_test.c
 *
 * Support for sourcesink device
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "sourcesink_ext_core.h"
#include <linux/module.h>
#include <linux/slab.h>

struct testcase_handler {
	int (* func)(struct sse_usbtest_dev *dev,
		struct sse_usbtest_param_32 *param);
};

static int testcase0(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	unused(dev);
	unused(param);

	pr_info("TEST 0:  NOP\n");
	return 0;
}

/* Simple non-queued bulk I/O tests */
static int testcase1(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	struct urb *urb = NULL;
	struct usb_device *udev = testdev_to_usbdev(dev);
	int ret;

	if (dev->pipe[SS_BULK_OUT_EP_IDX] == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 1: write %d bytes %u times, pipe %d\n",
		param->length, param->iterations, dev->pipe[SS_BULK_OUT_EP_IDX]);
	urb = simple_alloc_urb_no_dma(udev, dev->pipe[SS_BULK_OUT_EP_IDX],
			param->length, 0);
	if (!urb)
		return -ENOMEM;

	/* FIRMWARE:  bulk sink (maybe accepts short writes) */
	ret = simple_io(dev, urb, param->iterations, 0, 0, "test1");
	simple_free_urb(urb);

	return ret;
}

static int testcase2(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	struct urb *urb = NULL;
	struct usb_device *udev = testdev_to_usbdev(dev);
	int ret;

	if (dev->pipe[SS_BULK_IN_EP_IDX] == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 2: read %d bytes %u times pipe %d\n",
			param->length, param->iterations, dev->pipe[SS_BULK_IN_EP_IDX]);
	urb = simple_alloc_urb_no_dma(udev, dev->pipe[SS_BULK_IN_EP_IDX],
			param->length, 0);
	if (!urb)
		return -ENOMEM;

	/* FIRMWARE:  bulk source (maybe generates short writes) */
	ret = simple_io(dev, urb, param->iterations, 0, 0, "test2");
	simple_free_urb(urb);

	return ret;
}

static int testcase3(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	struct urb *urb = NULL;
	struct usb_device *udev = testdev_to_usbdev(dev);
	int ret;

	if (dev->pipe[SS_BULK_OUT_EP_IDX] == 0 || param->vary == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 3: write/%d 0..%d bytes %u times\n",
			param->vary, param->length, param->iterations);
	urb = simple_alloc_urb_no_dma(udev, dev->pipe[SS_BULK_OUT_EP_IDX],
			param->length, 0);
	if (!urb)
		return -ENOMEM;

	/* FIRMWARE:  bulk sink (maybe accepts short writes) */
	ret = simple_io(dev, urb, param->iterations, param->vary, 0, "test3");
	simple_free_urb(urb);

	return ret;
}

static int testcase4(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	struct urb *urb = NULL;
	struct usb_device *udev = testdev_to_usbdev(dev);
	int ret;

	if (dev->pipe[SS_BULK_IN_EP_IDX] == 0 || param->vary == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 4: read/%d 0..%d bytes %u times\n",
			param->vary, param->length, param->iterations);
	urb = simple_alloc_urb_no_dma(udev, dev->pipe[SS_BULK_IN_EP_IDX],
			param->length, 0);
	if (!urb)
		return -ENOMEM;
	/* FIRMWARE:  bulk source (maybe generates short writes) */
	ret = simple_io(dev, urb, param->iterations, param->vary, 0, "test4");
	simple_free_urb(urb);

	return ret;
}

/* Queued bulk I/O tests */
static int testcase5(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	struct scatterlist *sg = NULL;
	int ret;
	struct usb_sg_request req;

	if (dev->pipe[SS_BULK_OUT_EP_IDX] == 0 || param->sglen == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 5: write %d sglists %d entries of %d bytes\n",
			param->iterations,
			param->sglen, param->length);
	sg = alloc_sglist(param->sglen, param->length,
			0, dev, dev->pipe[SS_BULK_OUT_EP_IDX]);
	if (!sg)
		return -ENOMEM;

	/* FIRMWARE:  bulk sink (maybe accepts short writes) */
	ret = perform_sglist(dev, param->iterations, dev->pipe[SS_BULK_OUT_EP_IDX],
			&req, sg, param->sglen);
	free_sglist(sg, param->sglen);

	return ret;
}

static int testcase6(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	struct scatterlist *sg = NULL;
	int ret;
	struct usb_sg_request req;

	if (dev->pipe[SS_BULK_IN_EP_IDX] == 0 || param->sglen == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 6: read %d sglists %d entries of %d bytes\n",
			param->iterations,
			param->sglen, param->length);
	sg = alloc_sglist(param->sglen, param->length,
			0, dev, dev->pipe[SS_BULK_IN_EP_IDX]);
	if (!sg)
		return -ENOMEM;

	/* FIRMWARE:  bulk source (maybe generates short writes) */
	ret = perform_sglist(dev, param->iterations, dev->pipe[SS_BULK_IN_EP_IDX],
			&req, sg, param->sglen);
	free_sglist(sg, param->sglen);

	return ret;
}

static int testcase7(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	struct scatterlist *sg = NULL;
	int ret;
	struct usb_sg_request req;

	if (dev->pipe[SS_BULK_OUT_EP_IDX] == 0 || param->sglen == 0 || param->vary == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 7: write/%d %d sglists %d entries 0..%d bytes\n",
			param->vary, param->iterations,
			param->sglen, param->length);
	sg = alloc_sglist(param->sglen, param->length,
			param->vary, dev, dev->pipe[SS_BULK_OUT_EP_IDX]);
	if (!sg)
		return -ENOMEM;
	/* FIRMWARE:  bulk sink (maybe accepts short writes) */
	ret = perform_sglist(dev, param->iterations, dev->pipe[SS_BULK_OUT_EP_IDX],
			&req, sg, param->sglen);
	free_sglist(sg, param->sglen);

	return ret;
}

static int testcase8(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	struct scatterlist *sg = NULL;
	int ret;
	struct usb_sg_request req;

	if (dev->pipe[SS_BULK_IN_EP_IDX] == 0 || param->sglen == 0 || param->vary == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 8: read/%d %d sglists %d entries 0..%d bytes\n",
			param->vary, param->iterations,
			param->sglen, param->length);
	sg = alloc_sglist(param->sglen, param->length,
			param->vary, dev, dev->pipe[SS_BULK_IN_EP_IDX]);
	if (!sg)
		return -ENOMEM;

	/* FIRMWARE:  bulk source (maybe generates short writes) */
	ret = perform_sglist(dev, param->iterations, dev->pipe[SS_BULK_IN_EP_IDX],
			&req, sg, param->sglen);
	free_sglist(sg, param->sglen);

	return ret;
}

/* non-queued sanity tests for control (chapter 9 subset) */
static int testcase9(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	int ret = 0;
	u32 i;

	pr_info("TEST 9: ch9 (subset) control tests, %d times\n",
			param->iterations);
	for (i = param->iterations; ret == 0 && i--;) /* NOP */
		ret = ch9_postconfig(dev);

	if (ret)
		pr_err("ch9 subset failed, "
				"iterations left %d\n", i);

	return ret;
}

/* queued control messaging */
static int testcase10(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	pr_info("TEST 10: queue %d control calls, %d times\n",
			param->sglen,
			param->iterations);
	return test_ctrl_queue(dev, param);
}

/* simple non-queued unlinks (ring with one urb) */
static int testcase11(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	int ret = 0;
	u32 i;

	if (dev->pipe[SS_BULK_IN_EP_IDX] == 0 || !param->length)
		return -EOPNOTSUPP;

	pr_info("TEST 11: unlink %d reads of %d\n",
			param->iterations, param->length);
	for (i = param->iterations; ret == 0 && i--;) /* NOP */
		ret = unlink_simple(dev, dev->pipe[SS_BULK_IN_EP_IDX],
					param->length, 0);
	if (ret)
		pr_err("unlink reads failed %d, "
			"iterations left %d\n", ret, i);

	return ret;
}

static int testcase12(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	int ret = 0;
	u32 i;

	if (dev->pipe[SS_BULK_OUT_EP_IDX] == 0 || !param->length)
		return -EOPNOTSUPP;

	pr_info("TEST 12: unlink %d writes of %d\n",
			param->iterations, param->length);
	for (i = param->iterations; ret == 0 && i--;) /* NOP */
		ret = unlink_simple(dev, dev->pipe[SS_BULK_OUT_EP_IDX],
					param->length, 0);
	if (ret)
		pr_err("unlink writes failed %d, "
			"iterations left %d\n", ret, i);

	return ret;
}

/* ep halt tests */
static int testcase13(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	int ret = 0;
	u32 i;

	if ((dev->pipe[SS_BULK_OUT_EP_IDX] == 0 &&
		dev->pipe[SS_BULK_IN_EP_IDX] == 0) || param->length == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 13: set/clear %d halts, len %u\n",
			param->iterations, param->length);
	for (i = param->iterations; ret == 0 && i--;) /* NOP */
		ret = halt_simple(dev, true, param->length);

	if (ret)
		pr_err("halts failed, iterations left %d\n", i);

	return ret;
}

/* control write tests */
static int testcase14(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	pr_info("TEST 14: %d ep0out, %d..%d vary %d\n",
			param->iterations,
			1, param->length,
			param->vary);
	return ctrl_out(dev, param->iterations,
			param->length, param->vary, 0, false);
}

/* iso write tests */
static int testcase15(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	if (dev->pipe[SS_ISOC_OUT_EP_IDX] == 0 || param->sglen == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 15: write %d iso, %d entries of %d bytes\n",
			param->iterations,
			param->sglen, param->length);
	/* FIRMWARE:  iso sink */
	return test_queue(dev, param,
			dev->pipe[SS_ISOC_OUT_EP_IDX],
			dev->desc[SS_ISOC_OUT_EP_IDX], 0);
}

/* iso read tests */
static int testcase16(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	if (dev->pipe[SS_ISOC_IN_EP_IDX] == 0 || param->sglen == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 16: read %d iso, %d entries of %d bytes\n",
			param->iterations,
			param->sglen, param->length);
	/* FIRMWARE:  iso source */
	return test_queue(dev, param,
			dev->pipe[SS_ISOC_IN_EP_IDX],
			dev->desc[SS_ISOC_IN_EP_IDX], 0);
}

/* scatterlist cancel (needs helper thread) */
/* Tests for bulk I/O using DMA mapping by core and odd address */
static int testcase17(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	if (dev->pipe[SS_BULK_OUT_EP_IDX] == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 17: write odd addr %d bytes %u times core map\n",
			param->length, param->iterations);

	return test_unaligned_bulk(
			dev, dev->pipe[SS_BULK_OUT_EP_IDX],
			param->length, param->iterations,
			0, "test17");
}

static int testcase18(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	if (dev->pipe[SS_BULK_IN_EP_IDX] == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 18: read odd addr %d bytes %u times core map\n",
			param->length, param->iterations);

	return test_unaligned_bulk(
			dev, dev->pipe[SS_BULK_IN_EP_IDX],
			param->length, param->iterations,
			0, "test18");
}

/* Tests for bulk I/O using premapped coherent buffer and odd address */
static int testcase19(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	if (dev->pipe[SS_BULK_OUT_EP_IDX] == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 19: write odd addr %d bytes %u times premapped\n",
			param->length, param->iterations);

	return test_unaligned_bulk(
			dev, dev->pipe[SS_BULK_OUT_EP_IDX],
			param->length, param->iterations,
			URB_NO_TRANSFER_DMA_MAP, "test19");
}

static int testcase20(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	if (dev->pipe[SS_BULK_IN_EP_IDX] == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 20: read odd addr %d bytes %u times premapped\n",
			param->length, param->iterations);

	return test_unaligned_bulk(
			dev, dev->pipe[SS_BULK_IN_EP_IDX],
			param->length, param->iterations,
			URB_NO_TRANSFER_DMA_MAP, "test20");
}

/* control write tests with unaligned buffer */
static int testcase21(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	pr_info("TEST 21: %d ep0out odd addr, %d..%d vary %d\n",
			param->iterations,
			1, param->length,
			param->vary);
	return ctrl_out(dev, param->iterations,
			param->length, param->vary, 1, false);
}

/* unaligned iso tests */
static int testcase22(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	if (dev->pipe[SS_ISOC_OUT_EP_IDX] == 0 || param->sglen == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 22: write %d iso odd, %d entries of %d bytes\n",
			param->iterations,
			param->sglen, param->length);
	return test_queue(dev, param,
			dev->pipe[SS_ISOC_OUT_EP_IDX],
			dev->desc[SS_ISOC_OUT_EP_IDX], 1);
}

static int testcase23(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	if (dev->pipe[SS_ISOC_IN_EP_IDX] == 0 || param->sglen == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 23: read %d iso odd, %d entries of %d bytes\n",
			param->iterations,
			param->sglen, param->length);
	return test_queue(dev, param,
			dev->pipe[SS_ISOC_IN_EP_IDX],
			dev->desc[SS_ISOC_IN_EP_IDX], 1);
}

/* unlink URBs from a bulk-OUT queue */
static int testcase24(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	int ret = 0;
	u32 i;

	/* mini param->sglen is 4 */
	if (dev->pipe[SS_BULK_OUT_EP_IDX] == 0 || !param->length || param->sglen < 4)
		return -EOPNOTSUPP;

	pr_info("TEST 24: unlink from %d queues of "
			"%d %d-byte writes\n",
			param->iterations, param->sglen, param->length);
	for (i = param->iterations; ret == 0 && i > 0; --i) {
		ret = unlink_queued(dev, dev->pipe[SS_BULK_OUT_EP_IDX],
					param->sglen, param->length);
		if (ret) {
			pr_err(
				"unlink queued writes failed %d, "
				"iterations left %d\n", ret, i);
			return ret;
		}
	}
	return ret;
}

/* Simple non-queued interrupt I/O tests */
static int testcase25(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	struct urb *urb = NULL;
	struct usb_device *udev = testdev_to_usbdev(dev);
	int ret;

	if (dev->pipe[SS_INTR_OUT_EP_IDX] == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 25: write %d bytes %u times\n",
			param->length, param->iterations);
	urb = simple_alloc_urb_no_dma(udev, dev->pipe[SS_INTR_OUT_EP_IDX], param->length,
			dev->desc[SS_INTR_OUT_EP_IDX]->bInterval);
	if (!urb)
		return -ENOMEM;

	/* FIRMWARE: interrupt sink (maybe accepts short writes) */
	ret = simple_io(dev, urb, param->iterations, 0, 0, "test25");
	simple_free_urb(urb);

	return ret;
}

static int testcase26(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	struct urb *urb = NULL;
	struct usb_device *udev = testdev_to_usbdev(dev);
	int ret;

	if (dev->pipe[SS_INTR_IN_EP_IDX] == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 26: read %d bytes %u times\n",
			param->length, param->iterations);
	urb = simple_alloc_urb_no_dma(udev, dev->pipe[SS_INTR_IN_EP_IDX], param->length,
			dev->desc[SS_INTR_IN_EP_IDX]->bInterval);
	if (!urb)
		return -ENOMEM;
	/* FIRMWARE: interrupt source (maybe generates short writes) */
	ret = simple_io(dev, urb, param->iterations, 0, 0, "test26");
	simple_free_urb(urb);

	return ret;
}

static int testcase27(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	/* We do performance test, so ignore data compare */
	if (dev->pipe[SS_BULK_OUT_EP_IDX] == 0 || param->sglen == 0
			|| dev->info->pattern < 2) /* mini pattern is 2 */
		return -EOPNOTSUPP;

	pr_info("TEST 27: bulk write %dMbytes\n", (param->iterations *
				param->sglen * param->length) / (1024 * 1024)); /* 1024: mod to Mb */
	return test_queue(dev, param,
			dev->pipe[SS_BULK_OUT_EP_IDX], NULL, 0);
}

static int testcase28(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	if (dev->pipe[SS_BULK_IN_EP_IDX] == 0 || param->sglen == 0
			|| dev->info->pattern < 2) /* mini pattern is 2 */
		return -EOPNOTSUPP;

	pr_info("TEST 28: bulk read %dMbytes\n", (param->iterations *
				param->sglen * param->length) / (1024 * 1024)); /* 1024: mod to Mb */

	return test_queue(dev, param,
			dev->pipe[SS_BULK_IN_EP_IDX], NULL, 0);
}

/* halt intr ep */
static int testcase29(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	int ret = 0;
	u32 i;

	if ((dev->pipe[SS_INTR_OUT_EP_IDX] == 0 &&
		dev->pipe[SS_INTR_IN_EP_IDX] == 0) || param->length == 0)
		return -EOPNOTSUPP;

	pr_info("TEST 29: set/clear int %d halts len %u\n",
			param->iterations, param->length);
	for (i = param->iterations; ret == 0 && i--;) /* NOP */
		ret = halt_simple(dev, false, param->length);

	if (ret)
		pr_err("halts failed, iterations left %d\n", i);

	return ret;
}

/* intr, simple non-queued unlinks (ring with one urb) */
static int testcase30(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	int ret = 0;
	u32 i;

	if (dev->pipe[SS_INTR_IN_EP_IDX] == 0 || !param->length)
		return -EOPNOTSUPP;

	pr_info("TEST 30: unlink %d reads of %d\n",
			param->iterations, param->length);
	for (i = param->iterations; ret == 0 && i--;) /* NOP */
		ret = unlink_simple(dev, dev->pipe[SS_INTR_IN_EP_IDX],
					param->length,
					dev->desc[SS_INTR_IN_EP_IDX]->bInterval);
	if (ret)
		pr_err("unlink reads failed %d, "
			"iterations left %d\n", ret, i);

	return ret;
}

static int testcase31(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	int ret = 0;
	u32 i;

	if (dev->pipe[SS_INTR_OUT_EP_IDX] == 0 || !param->length)
		return -EOPNOTSUPP;

	pr_info("TEST 31: unlink %d writes of %d\n",
			param->iterations, param->length);
	for (i = param->iterations; ret == 0 && i--;) /* NOP */
		ret = unlink_simple(dev, dev->pipe[SS_INTR_OUT_EP_IDX],
					param->length,
					dev->desc[SS_INTR_OUT_EP_IDX]->bInterval);
	if (ret)
		pr_err("unlink writes failed %d, "
			"iterations left %d\n", ret, i);

	return ret;
}

static int testcase32(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param)
{
	pr_info("TEST 32: %d ep0out, %d..%d vary %d\n",
			param->iterations,
			1, param->length,
			param->vary);
	return ctrl_out(dev, param->iterations,
			param->length, param->vary, 0, true);
}

static struct testcase_handler testcases[] = {
	{.func = testcase0},
	{.func = testcase1},
	{.func = testcase2},
	{.func = testcase3},
	{.func = testcase4},
	{.func = testcase5},
	{.func = testcase6},
	{.func = testcase7},
	{.func = testcase8},
	{.func = testcase9},
	{.func = testcase10},
	{.func = testcase11},
	{.func = testcase12},
	{.func = testcase13},
	{.func = testcase14},
	{.func = testcase15},
	{.func = testcase16},
	{.func = testcase17},
	{.func = testcase18},
	{.func = testcase19},
	{.func = testcase20},
	{.func = testcase21},
	{.func = testcase22},
	{.func = testcase23},
	{.func = testcase24},
	{.func = testcase25},
	{.func = testcase26},
	{.func = testcase27},
	{.func = testcase28},
	{.func = testcase29},
	{.func = testcase30},
	{.func = testcase31},
	{.func = testcase32}
};

/* Run tests. */
static int sse_usbtest_do_ioctl(struct sse_usbtest_dev *dev,
				struct sse_usbtest_param_32 *param)
{
	int	retval = -EOPNOTSUPP;

	if (param->iterations <= 0)
		return -EINVAL;
	if (param->sglen > MAX_SGLEN)
		return -EINVAL;
	/*
	 * Just a bunch of test cases that every HCD is expected to handle.
	 *
	 * Some may need specific firmware, though it'd be good to have
	 * one firmware image to handle all the test cases.
	 *
	 * add more tests!  cancel requests, verify the data, control
	 * queueing, concurrent read+write threads, and so on.
	 */
	if (param->test_num < ARRAY_SIZE(testcases))
		retval = testcases[param->test_num].func(dev, param);

	return retval;
}

static void handler_work(struct work_struct *work)
{
	struct timespec64 start;
	struct timespec64 end;
	struct timespec64 duration;
	struct sse_usbtest_dev *dev = container_of(work,
				    struct sse_usbtest_dev, handler_work);
	int retval;

	ktime_get_ts64(&start);

	retval = sse_usbtest_do_ioctl(dev, &dev->param);

	ktime_get_ts64(&end);

	duration = timespec64_sub(end, start);

	dev->param.duration_sec = duration.tv_sec;
	dev->param.duration_usec = duration.tv_nsec / NSEC_PER_USEC;
	dev->param.ret = retval;
}

/*
 * We only have this one interface to user space, through usbfs.
 * User mode code can scan usbfs to find N different devices (maybe on
 * different busses) to use when testing, and allocate one thread per
 * test.  So discovery is simplified, and we have no device naming issues.
 *
 * Don't use these only as stress/load tests.  Use them along with with
 * other USB bus activity:  plugging, unplugging, mousing, mp3 playback,
 * video capture, and so on.  Run different tests at different times, in
 * different sequences.  Nothing here should interact with other devices,
 * except indirectly by consuming USB bandwidth and CPU resources for test
 * threads and request completion.  But the only way to know that for sure
 * is to test when HC queues are in use by many devices.
 *
 * WARNING:  Because usbfs grabs udev->dev.sem before calling this ioctl(),
 * it locks out usbcore in certain code paths.  Notably, if you disconnect
 * the device-under-test, hub_wq will wait block forever waiting for the
 * ioctl to complete ... so that usb_disconnect() can abort the pending
 * urbs and then call sse_usbtest_disconnect().  To abort a test, you're best
 * off just killing the userspace task and waiting for it to exit.
 */
static int sse_usbtest_ioctl(struct usb_interface *intf,
				unsigned int code, void *buf)
{
	struct sse_usbtest_dev *dev = usb_get_intfdata(intf);
	struct sse_usbtest_param_64 *param_64 = buf;
	struct sse_usbtest_param_32 temp;
	struct sse_usbtest_param_32 *param_32 = buf;
	int retval = -EOPNOTSUPP;

	if (mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;

	if (code == SSE_USBTEST_REQUEST_SET_HALT) {
		retval = sse_set_clear_halt(dev, interface_to_usbdev(intf),
				*(uint32_t *)buf, true);
		goto free_mutex;
	}

	if (code == SSE_USBTEST_REQUEST_CLEAR_HALT) {
		retval = sse_set_clear_halt(dev, interface_to_usbdev(intf),
				*(uint32_t *)buf, false);
		goto free_mutex;
	}

	if (work_busy(&dev->handler_work)) {
		pr_err("handler_work is busy\n");
		retval = -EBUSY;
		goto free_mutex;
	}

	if (code == SSE_USBTEST_REQUEST_ALT) {
		dev->info->alt = *((int *)buf);
		retval = 0;
		goto free_mutex;
	}

	if (code == SSE_USBTEST_REQUEST_PATTERN) {
		dev->info->pattern = *((uint32_t *)buf);
		retval = 0;
		goto free_mutex;
	}

	/*
	 * if need a non-default altsetting to have any active endpoints.
	 * some tests change altsettings,
	 * force a default so most tests don't need to check.
	 */
	if (dev->info->alt >= 0 && dev->info->alt != dev->cur_alt) {
		pr_info("set altsetting %d\n", dev->info->alt);
		retval = set_altsetting(dev, dev->info->alt);
		if (retval) {
			pr_err("set altsetting to %d failed, %d\n",
					dev->info->alt, retval);
			goto free_mutex;
		}
		dev->cur_alt = intf->cur_altsetting->desc.bAlternateSetting;
		retval = get_endpoints(dev, intf);
		if (retval < 0) {
			pr_err("could not get ep %d\n", retval);
			goto free_mutex;
		}
	}

	switch (code) {
	case SSE_USBTEST_REQUEST_64:
		temp.test_num = param_64->test_num;
		temp.iterations = param_64->iterations;
		temp.length = param_64->length;
		temp.sglen = param_64->sglen;
		temp.vary = param_64->vary;
		param_32 = &temp;
		break;

	case SSE_USBTEST_REQUEST_32:
		break;

	default:
		retval = -EOPNOTSUPP;
		goto free_mutex;
	}

	/* sync operation */
	/* return test result to user space */
	if (param_32->test_num == SSE_GET_RES_CASE_NUM) {
		param_32->ret = dev->param.ret;
		param_32->duration_sec = dev->param.duration_sec;
		param_32->duration_usec = dev->param.duration_usec;
		retval = 0;
		goto free_mutex;
	}

	/* async operation, handle in work */
	memcpy(&dev->param, param_32, sizeof(dev->param));
	/* set ret defalut value as invalid, will be set ret value in workqueue */
	dev->param.ret = 0xff;

	if (queue_work(system_power_efficient_wq, &dev->handler_work)) {
		retval = 0;
		goto free_mutex;
	} else {
		pr_err("last case has not finished\n");
		retval = -EBUSY;
		goto free_mutex;
	}

free_mutex:
	mutex_unlock(&dev->lock);
	return retval;
}

static int sse_usbtest_probe(struct usb_interface *intf,
			const struct usb_device_id *id)
{
	struct usb_device *udev = NULL;
	struct sse_usbtest_dev *dev = NULL;
	struct sse_usbtest_info *info = NULL;
	int			status;

	udev = interface_to_usbdev(intf);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	info = (struct sse_usbtest_info *)id->driver_info;
	dev->info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!dev->info) {
		kfree(dev);
		return  -ENOMEM;
	}
	memcpy(dev->info, info, sizeof(*info));

	mutex_init(&dev->lock);

	dev->intf = intf;

	/* cacheline-aligned scratch for i/o */
	dev->buf = kmalloc(TBUF_SIZE, GFP_KERNEL);
	if (dev->buf == NULL) {
		kfree(dev->info);
		kfree(dev);
		return -ENOMEM;
	}

	/* NOTE this doesn't yet test the handful of difference that are
	 * visible with high speed interrupts:  bigger maxpacket (1K) and
	 * "high bandwidth" modes (up to 3 packets/uframe).
	 */
	dev->cur_alt = intf->cur_altsetting->desc.bAlternateSetting;
	status = get_endpoints(dev, intf);
	if (status < 0)
		pr_err("could not get ep %d\n", status);

	usb_set_intfdata(intf, dev);
	INIT_WORK(&dev->handler_work, handler_work);
	pr_info("%s tests alt %d\n", dev->info->name, dev->info->alt);
	return 0;
}

static int sse_usbtest_suspend(struct usb_interface *intf, pm_message_t message)
{
	return 0;
}

static int sse_usbtest_resume(struct usb_interface *intf)
{
	return 0;
}


static void sse_usbtest_disconnect(struct usb_interface *intf)
{
	struct sse_usbtest_dev *dev = usb_get_intfdata(intf);

	cancel_work_sync(&dev->handler_work);
	usb_set_intfdata(intf, NULL);
	pr_info("disconnect\n");
	kfree(dev->info);
	kfree(dev);
}

static struct sse_usbtest_info _sse_info_ = {
	.name		= "SourceSink Ext Test",
	.pattern	= 1, /* default: 1 */
	.alt		= 3, /* default: 3 */
};

static const struct usb_device_id id_table[] = {
	{ USB_DEVICE(0x12d1, 0xae86),
		.driver_info = (unsigned long)&_sse_info_,
	},
	{ }
};
MODULE_DEVICE_TABLE(usb, id_table);

static struct usb_driver sse_usbtest_driver = {
	.name =		"sourcesink_ext_test",
	.id_table =	id_table,
	.probe =	sse_usbtest_probe,
	.unlocked_ioctl = sse_usbtest_ioctl,
	.disconnect =	sse_usbtest_disconnect,
	.suspend =	sse_usbtest_suspend,
	.resume =	sse_usbtest_resume,
};

module_usb_driver(sse_usbtest_driver);
