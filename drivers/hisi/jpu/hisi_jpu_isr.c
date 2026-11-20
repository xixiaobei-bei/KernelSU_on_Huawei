/*
 * jpeg jpu isr
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

#include <linux/delay.h>
#include "hisi_jpu.h"
#include "hisi_jpu_def.h"
#include "jpgdec_platform.h"

#define JPU_DEC_DONE_TIMEOUT_THRESHOLD_ASIC 400
#define JPU_DEC_DONE_TIMEOUT_THRESHOLD_FPGA (15 * 1000)

static void hisi_jpu_dec_done_isr_handler(struct hisi_jpu_data_type *hisijd)
{
	hisijd->jpu_dec_done_flag = 1;

	wake_up_interruptible_all(&hisijd->jpu_dec_done_wq);
}

int hisi_jpu_dec_done_config(struct hisi_jpu_data_type *hisijd,
	const struct jpu_data_t *jpu_req)
{
	int ret;
	uint32_t timeout_interval;
	int times = 0;
	unsigned long timeout_jiffies;

	jpu_check_null_return(hisijd, -1);

	if (hisijd->fpga_flag != 0)
		timeout_interval = JPU_DEC_DONE_TIMEOUT_THRESHOLD_FPGA;
	else
		timeout_interval = JPU_DEC_DONE_TIMEOUT_THRESHOLD_ASIC;

REDO_0:
	timeout_jiffies = (unsigned long)msecs_to_jiffies(timeout_interval);
	/*lint -e578*/
	ret = wait_event_interruptible_timeout(hisijd->jpu_dec_done_wq,
		hisijd->jpu_dec_done_flag, timeout_jiffies);
	if (ret == -ERESTARTSYS) {
		if (times < 50) { /* 50 times */
			times++;
			mdelay(10); /* 10 ms */
			goto REDO_0;
		}
	}

	hisijd->jpu_dec_done_flag = 0;
	if (ret <= 0) {
		hisi_jpu_err("wait_for jpu_dec_done_flag timeout ret = %d, "
			"jpu_dec_done_flag = %u\n", ret, hisijd->jpu_dec_done_flag);
		ret = -ETIMEDOUT;
	} else {
		hisi_jpu_info("finish decode jpu_dec_done_flag = %u\n",
			hisijd->jpu_dec_done_flag);
		hisi_jpu_dec_normal_reset(hisijd);
		ret = 0;
	}

	return ret;
}

int hisi_jpu_dec_err_config(struct hisi_jpu_data_type *hisijd)
{
	hisi_jpu_info("+\n");
	jpu_check_null_return(hisijd, -EINVAL);

	if (g_debug_jpu_dec != 0)
		hisi_jpu_err("jpu decode err\n");

	hisi_jpu_dec_error_reset(hisijd);
	hisi_jpu_info("-\n");
	return 0;
}

int hisi_jpu_dec_other_config(struct hisi_jpu_data_type *hisijd)
{
	hisi_jpu_info("+\n");
	jpu_check_null_return(hisijd, -EINVAL);

	hisi_jpu_dec_error_reset(hisijd);
	hisi_jpu_info("-\n");
	return 0;
}

static int hisi_jpu_dec_check_isr(const struct hisi_jpu_data_type *hisijd)
{
	char __iomem *jpu_top_base = NULL;

	jpu_check_null_return(hisijd, -1);
	jpu_top_base = hisijd->jpu_top_base;
	jpu_check_null_return(jpu_top_base, -1);

	return 0;
}

irqreturn_t hisi_jpu_dec_done_isr(int irq, void *ptr)
{
	struct hisi_jpu_data_type *hisijd = NULL;
	char __iomem *jpu_top_base = NULL;

	jpu_check_null_return(ptr, IRQ_HANDLED);
	hisijd = (struct hisi_jpu_data_type *)ptr;
	if (hisi_jpu_dec_check_isr(hisijd) != 0)
		goto err_out;

	jpu_top_base = hisijd->jpu_top_base;
	outp32(jpu_top_base + JPGDEC_IRQ_REG0, 0x1);
	hisi_jpu_dec_done_isr_handler(hisijd);

err_out:
	return IRQ_HANDLED;
}

irqreturn_t hisi_jpu_dec_err_isr(int irq, void *ptr)
{
	struct hisi_jpu_data_type *hisijd = NULL;
	char __iomem *jpu_top_base = NULL;

	hisi_jpu_info("+\n");
	jpu_check_null_return(ptr, IRQ_HANDLED);
	hisijd = (struct hisi_jpu_data_type *)ptr;
	if (hisi_jpu_dec_check_isr(hisijd) != 0)
		goto err_out;

	jpu_top_base = hisijd->jpu_top_base;
	outp32(jpu_top_base + JPGDEC_IRQ_REG0, 0x4); /* 0x4 set irq reg */
	if (hisi_jpu_dec_err_config(hisijd) != 0)
		hisi_jpu_err("hisi_jpu_dec_err_config failed\n");
	hisi_jpu_info("-\n");

err_out:
	return IRQ_HANDLED;
}

irqreturn_t hisi_jpu_dec_other_isr(int irq, void *ptr)
{
	struct hisi_jpu_data_type *hisijd = NULL;
	char __iomem *jpu_top_base = NULL;

	jpu_check_null_return(ptr, IRQ_HANDLED);
	hisijd = (struct hisi_jpu_data_type *)ptr;
	if (hisi_jpu_dec_check_isr(hisijd) != 0)
		goto err_out;

	jpu_top_base = hisijd->jpu_top_base;
	outp32(jpu_top_base + JPGDEC_IRQ_REG0, 0x8); /* 0x8 set irq reg */
	if (hisi_jpu_dec_other_config(hisijd) != 0)
		hisi_jpu_err("hisi_jpu_dec_other_config failed\n");

err_out:
	return IRQ_HANDLED;
}

void hisi_jpu_dec_interrupt_unmask(const struct hisi_jpu_data_type *hisijd)
{
	char __iomem *jpu_top_base = NULL;
	uint32_t unmask;

	if (hisi_jpu_dec_check_isr(hisijd) != 0)
		return;

	jpu_top_base = hisijd->jpu_top_base;
	unmask = ~0;
	unmask &= ~(BIT_JPGDEC_INT_DEC_ERR | BIT_JPGDEC_INT_DEC_FINISH);

	outp32(jpu_top_base + JPGDEC_IRQ_REG1, unmask);
}

void hisi_jpu_dec_interrupt_mask(const struct hisi_jpu_data_type *hisijd)
{
	char __iomem *jpu_top_base = NULL;
	uint32_t mask;

	if (hisi_jpu_dec_check_isr(hisijd) != 0)
		return;

	jpu_top_base = hisijd->jpu_top_base;

	mask = ~0;
	outp32(jpu_top_base + JPGDEC_IRQ_REG1, mask);
}

void hisi_jpu_dec_interrupt_clear(const struct hisi_jpu_data_type *hisijd)
{
	char __iomem *jpu_top_base = NULL;

	if (hisi_jpu_dec_check_isr(hisijd) != 0)
		return;
	jpu_top_base = hisijd->jpu_top_base;

	/* clear jpg decoder IRQ state
	 * [3]: jpgdec_int_over_time;
	 * [2]: jpgdec_int_dec_err;
	 * [1]: jpgdec_int_bs_res;
	 * [0]: jpgdec_int_dec_finish;
	 */
	outp32(jpu_top_base + JPGDEC_IRQ_REG0, 0xF); /* 0xF clera irq */
}

