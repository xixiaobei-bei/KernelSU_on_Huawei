/*
 * jpeg jpu utils
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "hisi_jpu_utils.h"
#include "linux/delay.h"
#include <linux/interrupt.h>
#include <media/camera/jpeg/jpeg_base.h>
#include "hisi_jpu.h"
#include "hisi_jpu_iommu.h"
#include "hisi_jpu_def.h"
#include "jpgdec_platform.h"

#define IRQ_JPU_DEC_DONE_NAME   "irq_jpu_dec_done"
#define IRQ_JPU_DEC_ERR_NAME    "irq_jpu_dec_err"
#define IRQ_JPU_DEC_OTHER_NAME  "irq_jpu_dec_other"
#define IRQ_JPU_DEC_MERGED_NAME "irq_jpu_dec_merged"

static void hisijpu_set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (uint32_t)((1UL << bw) - 1UL);
	uint32_t reg_tmp;

	reg_tmp = (uint32_t)inp32(addr);
	reg_tmp &= ~(mask << bs);

	outp32(addr, reg_tmp | ((val & mask) << bs));
}

static int hisijpu_requset_unmerged_irq(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;

	if (hisijd->jpu_done_irq != 0) {
		ret = request_irq(hisijd->jpu_done_irq, hisi_jpu_dec_done_isr, 0,
			IRQ_JPU_DEC_DONE_NAME, (void *)hisijd);
		if (ret != 0) {
			hisi_jpu_err("request_irq failed, irq_no = %u error = %d\n",
				hisijd->jpu_done_irq, ret);
			return ret;
		}
		disable_irq(hisijd->jpu_done_irq);
	}

	if (hisijd->jpu_err_irq != 0) {
		ret = request_irq(hisijd->jpu_err_irq, hisi_jpu_dec_err_isr, 0,
			IRQ_JPU_DEC_ERR_NAME, (void *)hisijd);
		if (ret != 0) {
			hisi_jpu_err("request_irq failed, irq_no = %u error = %d\n",
				hisijd->jpu_err_irq, ret);
			return ret;
		}
		disable_irq(hisijd->jpu_err_irq);
	}

	if (hisijd->jpu_other_irq != 0) {
		ret = request_irq(hisijd->jpu_other_irq, hisi_jpu_dec_other_isr, 0,
			IRQ_JPU_DEC_OTHER_NAME, (void *)hisijd);
		if (ret != 0) {
			hisi_jpu_err("request_irq failed, irq_no = %u error = %d\n",
				hisijd->jpu_other_irq, ret);
			return ret;
		}
		disable_irq(hisijd->jpu_other_irq);
	}

	return ret;
}

static int hisijpu_irq_request(struct hisi_jpu_data_type *hisijd)
{
	jpu_check_null_return(hisijd, -EINVAL);

	return hisijpu_requset_unmerged_irq(hisijd);
}

static void hisijpu_irq_free(struct hisi_jpu_data_type *hisijd)
{
	if (hisijd->jpu_done_irq != 0) {
		free_irq(hisijd->jpu_done_irq, 0);
		hisijd->jpu_done_irq = 0;
	}

	if (hisijd->jpu_err_irq != 0) {
		free_irq(hisijd->jpu_err_irq, 0);
		hisijd->jpu_err_irq = 0;
	}

	if (hisijd->jpu_other_irq != 0) {
		free_irq(hisijd->jpu_other_irq, 0);
		hisijd->jpu_other_irq = 0;
	}
}

static void hisijpu_irq_disable(struct hisi_jpu_data_type *hisijd)
{
	if (hisijd->jpu_err_irq != 0)
		disable_irq(hisijd->jpu_err_irq);
	if (hisijd->jpu_done_irq != 0)
		disable_irq(hisijd->jpu_done_irq);
	if (hisijd->jpu_other_irq != 0)
		disable_irq(hisijd->jpu_other_irq);
}

static int hisijpu_jpu_regulator_disable(
	const struct hisi_jpu_data_type *hisijd)
{
	int ret;

	hisi_jpu_debug("+\n");
	ret = regulator_disable(hisijd->jpu_regulator);
	if (ret != 0)
		hisi_jpu_err("jpu regulator_disable failed, error = %d\n", ret);

	hisi_jpu_debug("-\n");
	return ret;
}

int hisi_jpu_register(struct hisi_jpu_data_type *hisijd)
{
	int ret;

	jpu_check_null_return(hisijd, -EINVAL);
	jpu_check_null_return(hisijd->pdev, -EINVAL);
	hisijd->jpu_dec_done_flag = 0;
	init_waitqueue_head(&hisijd->jpu_dec_done_wq);

	sema_init(&hisijd->blank_sem, 1);
	hisijd->power_on = false;
	hisijd->jpu_res_initialized = false;

	ret = hisijpu_enable_iommu(hisijd);
	if (ret != 0) {
		dev_err(&hisijd->pdev->dev, "hisijpu_enable_iommu failed\n");
		return ret;
	}

	ret = hisi_jpu_lb_alloc(hisijd);
	if (ret != 0) {
		dev_err(&hisijd->pdev->dev, "hisi_jpu_lb_alloc failed\n");
		return ret;
	}

	ret = hisijpu_irq_request(hisijd);
	if (ret != 0) {
		dev_err(&hisijd->pdev->dev, "hisijpu_irq_request failed\n");
		return ret;
	}



	return 0;
}

int hisi_jpu_unregister(struct hisi_jpu_data_type *hisijd)
{
	int ret;

	jpu_check_null_return(hisijd, -EINVAL);
	jpu_check_null_return(hisijd->pdev, -EINVAL);



	hisijpu_irq_disable(hisijd);
	hisijpu_irq_free(hisijd);
	hisi_jpu_lb_free(hisijd);

	return 0;
}

static int hisijpu_check_reg_state(const char __iomem *addr, uint32_t val)
{
	uint32_t tmp;
	int delay_count = 0;
	bool is_timeout = true;

	while (1) {
		tmp = (uint32_t) inp32(addr);
		if (((tmp & val) == val) || (delay_count > 10)) { /* loop less 10 */
			is_timeout = (delay_count > 10) ? true : false;
			udelay(10); /* 10us */
			break;
		}

		udelay(10); /* 10us */
		++delay_count;
	}

	if (is_timeout) {
		hisi_jpu_err("fail to wait reg\n");
		return -1;
	}

	return 0;
}

void hisi_jpu_dec_normal_reset(const struct hisi_jpu_data_type *hisijd)
{
	int ret;

	if (hisijd == NULL) {
		hisi_jpu_err("hisijd is NULL\n");
		return;
	}

	hisijpu_set_reg(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_RD_CFG1, 0x1, 1, REG_SET_25_BIT);

	ret = hisijpu_check_reg_state(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_RD_CFG1, AXI_JPEG_NR_RD_STOP);
	if (ret != 0)
		hisi_jpu_err("fail to wait JPGDEC_CVDR_AXI_RD_CFG1\n");

	hisijpu_set_reg(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_RD_CFG1, 0x0, 1, REG_SET_25_BIT);
}

static void hisijpu_set_reg_cvdr(const struct hisi_jpu_data_type *hisijd,
	uint32_t val)
{
	hisijpu_set_reg(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_RD_CFG1, val, 1, REG_SET_25_BIT);

	hisijpu_set_reg(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_RD_CFG2, val, 1, REG_SET_25_BIT);

	hisijpu_set_reg(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_WR_CFG1, val, 1, REG_SET_25_BIT);

	hisijpu_set_reg(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_WR_CFG2, val, 1, REG_SET_25_BIT);
}

static void hisi_jpu_check_reg_state(struct hisi_jpu_data_type *hisijd)
{
	int ret;

	ret = hisijpu_check_reg_state(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_RD_CFG1, AXI_JPEG_NR_RD_STOP);
	jpu_err_if_cond(ret != 0, "fail to wait JPGDEC_CVDR_AXI_RD_CFG1\n");

	ret = hisijpu_check_reg_state(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_RD_CFG2, AXI_JPEG_NR_RD_STOP);
	jpu_err_if_cond(ret != 0, "fail to wait JPGDEC_CVDR_AXI_RD_CFG2\n");

	ret = hisijpu_check_reg_state(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_WR_CFG1, AXI_JPEG_NR_RD_STOP);
	jpu_err_if_cond(ret != 0, "fail to wait JPGDEC_CVDR_AXI_WR_CFG1\n");

	ret = hisijpu_check_reg_state(hisijd->jpu_cvdr_base +
		JPGDEC_CVDR_AXI_WR_CFG2, AXI_JPEG_NR_RD_STOP);
	jpu_err_if_cond(ret != 0, "fail to wait JPGDEC_CVDR_AXI_WR_CFG2\n");
}

void hisi_jpu_dec_error_reset(struct hisi_jpu_data_type *hisijd)
{
	int ret;
	if (hisijd == NULL) {
		hisi_jpu_err("hisijd is NULL\n");
		return;
	}

	/* step1 */
	if (hisijd->jpu_support_platform == HISI_DSS_V510_CS) {
		hisijpu_set_reg(hisijd->jpu_top_base + JPGDEC_PREF_STOP, 0x0, 1, 0);
	} else {
		hisijpu_set_reg(hisijd->jpu_top_base + JPGDEC_CRG_CFG1,
			JPEGDEC_TPSRAM_2PRF_TRA, REG_SET_32_BIT, 0);
	}

	hisijpu_set_reg_cvdr(hisijd, 0x1);

	/* step2 */
	if (hisijd->jpu_support_platform == HISI_DSS_V510_CS) {
		ret = hisijpu_check_reg_state(hisijd->jpu_top_base +
			JPGDEC_PREF_STOP, 0x10);

		jpu_err_if_cond(ret != 0, "fail to wait JPGDEC_PREF_STOP\n");
	} else {
		ret = hisijpu_check_reg_state(hisijd->jpu_top_base +
			JPGDEC_RO_STATE, 0x2);

		jpu_err_if_cond(ret != 0, "fail to wait JPGDEC_RO_STATE\n");
	}
	hisi_jpu_check_reg_state(hisijd);

	/* step3,read bit0 is 1 */
	hisijpu_set_reg(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 1, 1, 0);
	ret = hisijpu_check_reg_state(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 0x1);
	jpu_err_if_cond(ret != 0, "fail to wait JPGDEC_CRG_CFG1\n");

	/* step4 */
	hisijpu_set_reg_cvdr(hisijd, 0x0);
	hisijpu_set_reg(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 0x0, REG_SET_32_BIT, 0);
}

static int hisijpu_irq_enable(struct hisi_jpu_data_type *hisijd)
{
	if (hisijd->jpu_done_irq != 0)
		enable_irq(hisijd->jpu_done_irq);

	if (hisijd->jpu_err_irq != 0)
		enable_irq(hisijd->jpu_err_irq);

	if (hisijd->jpu_other_irq != 0)
		enable_irq(hisijd->jpu_other_irq);

	return 0;
}


static void hisijpu_smmu_on(struct hisi_jpu_data_type *hisijd)
{
	uint32_t fama_ptw_msb;
	int ret;

	hisi_jpu_debug("+\n");


	/*
	 * Set global mode:
	 * SMMU_SCR_S.glb_nscfg = 0x3
	 * SMMU_SCR_P.glb_prot_cfg = 0x0
	 * SMMU_SCR.glb_bypass = 0x0
	 */
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_SCR, 0x0, 1, 0);

	/* for performance Ptw_mid: 0x1d, Ptw_pf: 0x1 */
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_SCR, 0x1, REG_SET_4_BIT,
		REG_SET_16_BIT);
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_SCR, 0x1D, REG_SET_6_BIT,
		REG_SET_20_BIT);

	/*
	 * Set interrupt mode:
	 * Clear all interrupt state: SMMU_INCLR_NS = 0xFF
	 * Enable interrupt: SMMU_INTMASK_NS = 0x0
	 */
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_INTRAW_NS, 0xFF,
		REG_SET_32_BIT, 0);
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_INTMASK_NS, 0x0,
		REG_SET_32_BIT, 0);

	/*
	 * Set non-secure pagetable addr:
	 * SMMU_CB_TTBR0 = non-secure pagetable address
	 * Set non-secure pagetable type:
	 * SMMU_CB_TTBCR.cb_ttbcr_des= 0x1 (long descriptor)
	 */
	if (hisi_jpu_domain_get_ttbr(hisijd) == 0) {
		hisi_jpu_err("get ttbr failed\n");
		return;
	}
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_CB_TTBR0,
		(uint32_t)hisi_jpu_domain_get_ttbr(hisijd), REG_SET_32_BIT, 0);
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_CB_TTBCR, 0x1, 1, 0);

	/* FAMA configuration */
	fama_ptw_msb = (hisi_jpu_domain_get_ttbr(hisijd) >> SHIFT_32_BIT) &
		JPEGDEC_FAMA_PTW_MSB;
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_FAMA_CTRL0, JPEGDEC_FAMA_PTW_MID,
		REG_SET_14_BIT, 0);
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_FAMA_CTRL1,
		fama_ptw_msb, REG_SET_7_BIT, 0);
	hisi_jpu_debug("-\n");
}


static int hisijpu_clk_try_low_freq(const struct hisi_jpu_data_type *hisijd)
{
	int ret;

	ret = jpeg_dec_set_rate(hisijd->jpg_func_clk, JPGDEC_LOWLEVEL_CLK_RATE);
	if (ret != 0) {
		hisi_jpu_err("jpg_func_clk set clk failed, error = %d\n", ret);
		return -EINVAL;
	}

	ret = jpeg_dec_clk_prepare_enable(hisijd->jpg_func_clk);
	if (ret != 0) {
		hisi_jpu_err("jpg_func_clk clk_prepare failed, error = %d\n", ret);
		return -EINVAL;
	}

	return ret;
}

int hisijpu_clk_enable(struct hisi_jpu_data_type *hisijd)
{
	int ret;

	hisi_jpu_debug("+\n");
	jpu_check_null_return(hisijd->jpg_func_clk, -EINVAL);

	hisi_jpu_debug("jpg func clk default rate is: %ld\n",
		JPGDEC_DEFALUTE_CLK_RATE);

	ret = jpeg_dec_set_rate(hisijd->jpg_func_clk, JPGDEC_DEFALUTE_CLK_RATE);
	if (ret != 0) {
		hisi_jpu_err("jpg_func_clk set clk failed, error = %d\n", ret);
		goto TRY_LOW_FREQ;
	}

	ret = jpeg_dec_clk_prepare_enable(hisijd->jpg_func_clk);
	if (ret != 0) {
		hisi_jpu_err("jpg_func_clk clk_prepare failed, error = %d\n", ret);
		goto TRY_LOW_FREQ;
	}

	hisi_jpu_debug("-\n");
	return ret;

TRY_LOW_FREQ:
	return hisijpu_clk_try_low_freq(hisijd);
}

int hisijpu_clk_disable(const struct hisi_jpu_data_type *hisijd)
{
	int ret;

	hisi_jpu_debug("+\n");
	jpu_check_null_return(hisijd->jpg_func_clk, -EINVAL);

	jpeg_dec_clk_disable_unprepare(hisijd->jpg_func_clk);

	ret = jpeg_dec_set_rate(hisijd->jpg_func_clk, JPGDEC_POWERDOWN_CLK_RATE);
	if (ret != 0) {
		hisi_jpu_err("fail to set power down rate, ret = %d\n", ret);
		return -EINVAL;
	}

	hisi_jpu_debug("-\n");
	return ret;
}

static int hisijpu_media1_regulator_disable(
	const struct hisi_jpu_data_type *hisijd)
{
	int ret;

	hisi_jpu_debug("+\n");
	ret = regulator_disable(hisijd->media1_regulator);
	if (ret != 0)
		hisi_jpu_err("jpu media1 regulator_disable failed, error = %d\n", ret);

	hisi_jpu_debug("-\n");
	return ret;
}

static int hisijpu_media1_regulator_enable(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;

	hisi_jpu_debug("+\n");
	ret = regulator_enable(hisijd->media1_regulator);
	if (ret != 0)
		hisi_jpu_err("jpu media1_regulator failed, error = %d\n", ret);

	hisi_jpu_debug("-\n");
	return ret;
}

static int hisijpu_jpu_regulator_enable(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;

	hisi_jpu_debug("+\n");
	ret = regulator_enable(hisijd->jpu_regulator);
	if (ret != 0)
		hisi_jpu_err("jpu regulator_enable failed, error = %d\n", ret);

	hisi_jpu_debug("-\n");
	return ret;
}

static int hisi_jpu_on_v600(struct hisi_jpu_data_type *hisijd)
{
	int ret;
	/* step 1 mediasubsys enable */
	ret = hisijpu_media1_regulator_enable(hisijd);
	if (ret != 0) {
		hisi_jpu_err("media1_regulator_enable fail\n");
		return ret;
	}

	/* step 2 clk_enable */
	ret = hisijpu_clk_enable(hisijd);
	if (ret != 0) {
		hisi_jpu_err("hisijpu_clk_enable fail\n");
		hisijpu_media1_regulator_disable(hisijd);
		return ret;
	}

	/* step 3 regulator_enable ispsubsys */
	ret = hisijpu_jpu_regulator_enable(hisijd);
	if (ret != 0) {
		hisi_jpu_err("jpu_regulator_enable fail\n");
		hisijpu_clk_disable(hisijd);
		hisijpu_media1_regulator_disable(hisijd);
		return ret;
	}


	return 0;
}

int hisi_jpu_on(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;

	jpu_check_null_return(hisijd, -EINVAL);
	if (hisijd->power_on) {
		hisi_jpu_info("hisijd has already power_on\n");
		return ret;
	}

	ret = hisi_jpu_on_v600(hisijd);
	if (ret != 0)
		return ret;

	/* step 4 jpeg decoder inside clock enable */
	outp32(hisijd->jpu_top_base + JPGDEC_CRG_CFG0, 0x1);

	if (hisijd->jpu_support_platform == HISI_DSS_V400)
		outp32(hisijd->jpu_top_base + JPGDEC_MEM_CFG, 0x02605550);

	hisijpu_smmu_on(hisijd);

	hisi_jpu_dec_interrupt_mask(hisijd);
	hisi_jpu_dec_interrupt_clear(hisijd);

	ret = hisijpu_irq_enable(hisijd);
	if (ret != 0) {
		hisi_jpu_err("hisijpu_irq_enable failed\n");
		return -EINVAL;
	}

	hisi_jpu_dec_interrupt_unmask(hisijd);
	hisijd->power_on = true;

	return ret;
}

static int hisi_jpu_off_v600(struct hisi_jpu_data_type *hisijd)
{
	int ret;
	/* ispsubsys regulator disable */
	ret = hisijpu_jpu_regulator_disable(hisijd);
	if (ret != 0) {
		hisi_jpu_err("hisijpu_jpu_regulator_disable failed\n");
		return -EINVAL;
	}

	/* clk disable */
	ret = hisijpu_clk_disable(hisijd);
	if (ret != 0) {
		hisi_jpu_err("hisijpu_clk_disable failed\n");
		return -EINVAL;
	}

	/* media disable */
	ret = hisijpu_media1_regulator_disable(hisijd);
	if (ret != 0) {
		hisi_jpu_err("hisijpu_media1_regulator_disable failed\n");
		return -EINVAL;
	}

	return ret;
}

int hisi_jpu_off(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;

	jpu_check_null_return(hisijd, -EINVAL);
	if (!hisijd->power_on) {
		hisi_jpu_debug("hisijd has already power off\n");
		return ret;
	}


	hisi_jpu_dec_interrupt_mask(hisijd);
	hisijpu_irq_disable(hisijd);

	/* jpeg decoder inside clock disable */
	outp32(hisijd->jpu_top_base + JPGDEC_CRG_CFG0, 0x0);
	ret = hisi_jpu_off_v600(hisijd);
	if (ret != 0)
		return ret;
	hisijd->power_on = false;

	return ret;
}

#pragma GCC diagnostic pop
