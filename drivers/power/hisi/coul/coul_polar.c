/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: polar parameter calc for coul module
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

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <securec.h>
#include <partition_macro.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include "polar_table.h"
#include "coul_polar.h"
#include "coul_update_polar_info.h"

#define check_polar_dev_ops(_di, _name) \
	do { if (_di->polar_dev_ops->_name == NULL) return -EINVAL; } while (0)

#define POLAR_FILE_LIMIT      0660
#define POLAR_BUF_SHOW_LEN    128
#define POLAR_LUT_SIZE        0x1000
#define POLAR_LUT_OFFSET      0
static unsigned long g_polar_sample_interval[POLAR_TIME_ARRAY_NUM] = {
	500, 1500, 3000, 5000,
	10000, 15000,  25000, 35000,
	45000, 65000,  85000, 135000,
	225000, 375000, 625000, 1200000
};
static int g_polar_curr_interval[POLAR_CURR_ARRAY_NUM] = {
	200, 100, 150, 250, 400, 600, 1000, 2000
};
static int g_polar_curr_vector_interval[POLAR_CURR_ARRAY_VECTOR_NUM] = {
	200, 150, 400, 1000
};

/* ----Full-polarized OCV thermometer---- */
/*
 * soc(0-100): 2.5% (one level)
 * 55 degrees, 40 degrees, 25 degrees, 10 degrees,
 * 5 degrees, 0-5 degrees, 10 degrees, 20 degrees
 */
static int g_polar_temp_points[POLAR_OCV_PC_TEMP_COLS] = {
	55, 40, 25, 10, 5, 0, -5, -10, -20
};
static int g_polar_pc_points[POLAR_OCV_PC_TEMP_ROWS] = {
	1000, 975, 950, 925, 900,
	875, 850, 825, 800, 775,
	750, 725, 700, 675, 650,
	625, 600, 575, 550, 525,
	500, 475, 450, 425, 400,
	375, 350, 325, 300, 275,
	250, 225, 200, 175, 150,
	125, 100, 75, 50, 25, 0
};

static int g_polar_resistence_pc_points[POLAR_RES_PC_CURR_ROWS] = {
	950, 900, 850, 800, 750,
	700, 650, 600, 550, 500,
	450, 400, 350, 300, 250,
	200, 150, 100, 50, 0
};

static int g_polar_learn_temp_points[POLAR_LEARN_TEMP_COLS] = {
	55, 40, 25,
	24, 23, 22, 21, 20,
	19, 18, 17, 16, 15,
	14, 13, 12, 11, 10,
	9,  8,  7,  6, 5,
	4,  3,  2,  1, 0,
	-1, -2, -3, -4, -5,
	-6, -7, -8, -9, -10,
	-11, -12, -13, -14, -15,
	-16, -17, -18, -19, -20
};

static int g_polar_full_res_temp_points[POLAR_OCV_PC_TEMP_COLS] = {
	55, 40, 25, 10, 5, 0, -5, -10, -20
};
static struct hisi_polar_device *g_polar_di;
static struct polar_res_tbl g_polar_res_lut = {
	.rows = POLAR_OCV_PC_TEMP_COLS,
	.cols = POLAR_RES_PC_CURR_ROWS,
	.z_lens = POLAR_CURR_ARRAY_NUM,
	.x_array = g_polar_full_res_temp_points,
	.y_array = g_polar_resistence_pc_points,
	.z_array = g_polar_curr_interval,
};
static struct polar_res_tbl g_polar_res_lut0 = {
	.rows = POLAR_OCV_PC_TEMP_COLS,
	.cols = POLAR_RES_PC_CURR_ROWS,
	.z_lens = POLAR_CURR_ARRAY_NUM,
	.x_array = g_polar_full_res_temp_points,
	.y_array = g_polar_resistence_pc_points,
	.z_array = g_polar_curr_interval,
};
static struct polar_res_tbl g_polar_res_lut1 = {
	.rows = POLAR_OCV_PC_TEMP_COLS,
	.cols = POLAR_RES_PC_CURR_ROWS,
	.z_lens = POLAR_CURR_ARRAY_NUM,
	.x_array = g_polar_full_res_temp_points,
	.y_array = g_polar_resistence_pc_points,
	.z_array = g_polar_curr_interval,
};
static struct polar_res_tbl g_polar_res_lut5 = {
	.rows = POLAR_OCV_PC_TEMP_COLS,
	.cols = POLAR_RES_PC_CURR_ROWS,
	.z_lens = POLAR_CURR_ARRAY_NUM,
	.x_array = g_polar_full_res_temp_points,
	.y_array = g_polar_resistence_pc_points,
	.z_array = g_polar_curr_interval,
};
static struct polar_x_y_z_tbl g_polar_vector_lut = {
	.rows = POLAR_RES_PC_CURR_ROWS,
	.cols = POLAR_CURR_ARRAY_NUM,
	.z_lens = POLAR_OCV_PC_TEMP_COLS,
	.x_array = g_polar_resistence_pc_points,
	.y_array = g_polar_curr_interval,
	.z_array = g_polar_temp_points,
};
static struct polar_ocv_tbl g_polar_ocv_lut = {
	.rows = POLAR_OCV_PC_TEMP_ROWS,
	.cols = POLAR_OCV_PC_TEMP_COLS,
	.percent = g_polar_pc_points,
	.temp = g_polar_temp_points,
};

static struct polar_learn_tbl g_polar_learn_lut = {
	.rows = POLAR_RES_PC_CURR_ROWS,
	.cols = POLAR_LEARN_TEMP_COLS,
	.x_array = g_polar_resistence_pc_points,
	.y_array = g_polar_learn_temp_points,
};
static struct polar_curr_info g_polar_avg_curr_info
	[POLAR_TIME_ARRAY_NUM][POLAR_CURR_ARRAY_NUM + 1];
static long g_polar_err_a = POLAR_ERR_A_DEFAULT;
static long g_polar_err_b1;
static long g_polar_err_b2;

struct polar_err_a {
	long err_a;
	int temp;
};

static struct polar_err_a g_polar_err_a_array[POLAR_VALID_A_NUM] = { {0, 0} };
static int g_polar_err_a_coe[POLAR_VALID_A_NUM + 1] = { 25, 25, 25, 25 };

enum interplate_type {
	T1,
	T2,
	S1,
	S2,
	S,
	MAXYTYPE
};

enum SAMPLE_INFO_INT {
	I_FIF0,
	I_CUR,
	I_HEAD,
	I_LAST,
	I_MAX,
};

enum SAMPLE_INFO_LONG {
	L_SAMPLE,
	L_FLAG,
	L_MS,
	L_SYNC,
	L_DELTA,
	L_MAX,
};
struct hisi_polar_device *get_polar_dev(void)
{
	return g_polar_di;
}

struct polar_ocv_tbl *get_polar_ocv_lut(void)
{
	return &g_polar_ocv_lut;
}

struct polar_res_tbl *get_polar_res_lut(void)
{
	return &g_polar_res_lut;
}

struct polar_x_y_z_tbl *get_polar_vector_lut(void)
{
	return &g_polar_vector_lut;
}

struct polar_learn_tbl *get_polar_learn_lut(void)
{
	return &g_polar_learn_lut;
}

static int polar_dev_ops_check_fail(struct hisi_polar_device *di)
{
	if ((di == NULL) || (di->polar_dev_ops == NULL))
		return -EINVAL;
	check_polar_dev_ops(di, convert_regval2uah);
	check_polar_dev_ops(di, get_coul_time);
	check_polar_dev_ops(di, calculate_cc_uah);
	check_polar_dev_ops(di, get_eco_sample_flag);
	check_polar_dev_ops(di, set_eco_sample_flag);
	check_polar_dev_ops(di, clr_eco_data);
	return 0;
}

static int polar_linear_interpolate(int y0, int x0, int y1, int x1, int x)
{
	if ((y0 == y1) || (x == x0))
		return y0;
	if ((x1 == x0) || (x == x1))
		return y1;

	return y0 + ((y1 - y0) * (x - x0) / (x1 - x0));
}

/* Description: Find the value closest to x in the array */
static void interpolate_find_pos(const int *x_array, int rows, int x,
	int *row1, int *row2)
{
	int i;

	if (!x_array || !row1 || !row2)
		return;

	if (rows < 1)
		return;
	*row1 = 0;
	*row2 = 0;
	if (x > x_array[0])
		x = x_array[0];

	if (x < x_array[rows - 1])
		x = x_array[rows - 1];

	for (i = 0; i < rows; i++) {
		if (x == x_array[i]) {
			*row1 = i;
			*row2 = i;
			return;
		}
		if (x > x_array[i]) {
			if (!i)
				return;
			*row1 = i - 1;
			*row2 = i;
			break;
		}
	}
}

static void interpolate_find_pos_reverse(const int *x_array, int rows, int x,
	int *row1, int *row2)
{
	int i;

	if (!x_array || !row1 || !row2)
		return;

	if (rows < 1)
		return;
	*row1 = 0;
	*row2 = 0;
	if (x < x_array[0])
		x = x_array[0];

	if (x > x_array[rows - 1])
		x = x_array[rows - 1];

	for (i = 0; i < rows; i++) {
		if (x == x_array[i]) {
			*row1 = i;
			*row2 = i;
			return;
		}
		if (x < x_array[i]) {
			if (!i)
				return;
			*row1 = i - 1;
			*row2 = i;
			return;
		}
	}
}

static int interpolate_linear_x(int *x_array, const int *y_array, int rows, int x)
{
	int row1 = 0;
	int row2 = 0;
	int result;

	if ((!x_array) || (!y_array) || (x <= 0))
		return 0;
	if (rows < 1)
		return 0;

	interpolate_find_pos_reverse(x_array, rows, x, &row1, &row2);
	result = polar_linear_interpolate(y_array[row1], x_array[row1],
		y_array[row2], x_array[row2], x);
	return result;
}

int interpolate_two_dimension(const struct polar_res_tbl *lut,
	int x, int y, int z)
{
	int fac[MAXYTYPE] = {0};
	int row1 = 0;
	int row2 = 0;
	int col1 = 0;
	int col2 = 0;
	int z_index;
	int z_res[POLAR_CURR_ARRAY_NUM] = {0};

	if (!lut || (lut->rows < 1) || (lut->cols < 1))
		return 0;

	interpolate_find_pos(lut->x_array, lut->rows, x, &row1, &row2);
	interpolate_find_pos(lut->y_array, lut->cols, y, &col1, &col2);

	fac[T1] = POLAR_INT_COE;
	fac[S1] = POLAR_INT_COE;

	if (lut->x_array[row1] != lut->x_array[row2]) {
		fac[T1] = POLAR_INT_COE * (lut->x_array[row1] - x) /
			(lut->x_array[row1] - lut->x_array[row2]);
		fac[T2] = POLAR_INT_COE * (x - lut->x_array[row2]) /
			(lut->x_array[row1] - lut->x_array[row2]);
	}
	if (lut->y_array[col1] != lut->y_array[col2]) {
		fac[S1] = POLAR_INT_COE * (lut->y_array[col1] - y) /
			(lut->y_array[col1] - lut->y_array[col2]);
		fac[S2] = POLAR_INT_COE * (y - lut->y_array[col2]) /
			(lut->y_array[col1] - lut->y_array[col2]);
	}

	for (z_index = 0; z_index < lut->z_lens; z_index++) {
		fac[S] = lut->value[row2][col2][z_index] * fac[T1] * fac[S1] +
			lut->value[row2][col1][z_index] * fac[T1] * fac[S2] +
			lut->value[row1][col2][z_index] * fac[T2] * fac[S1] +
			lut->value[row1][col1][z_index] * fac[T2] * fac[S2];
		z_res[z_index] = fac[S] / (POLAR_INT_COE * POLAR_INT_COE);
	}

	fac[S] = interpolate_linear_x(lut->z_array, z_res, lut->z_lens, z);
	return fac[S];
}

static int interpolate_nearest_x(const int *x_array, int rows, int x)
{
	int row1 = 0;
	int row2 = 0;
	int index;

	if (!x_array)
		return 0;

	if (rows < 1)
		return 0;
	interpolate_find_pos(x_array, rows, x, &row1, &row2);
	if (x > (x_array[row1] + x_array[row2]) / 2) /* 2: average */
		index = row1;
	else
		index = row2;
	return index;
}

static int interpolate_curr_vector(const int *x_array, int rows, int x)
{
	int row1 = 0;
	int row2 = 0;
	int index;

	if (!x_array)
		return 0;

	if (rows < 1)
		return 0;

	interpolate_find_pos_reverse(x_array, rows, x, &row1, &row2);
	if (x > two_avg(x_array[row1], x_array[row2]))
		index = row2;
	else
		index = row1;

	return index;
}

#ifdef CONFIG_HISI_DEBUG_FS
/* look for ocv according to temp, lookup table and pc */
int interpolate_polar_ocv(struct polar_ocv_tbl *lut, int batt_temp_degc, int pc)
{
	int i, ocvrow1, ocvrow2, ocv;
	int row1 = 0;
	int row2 = 0;

	if (!lut) {
		polar_err("NULL point in [%s]\n", __func__);
		return -1;
	}

	if ((lut->rows <= 0) || (lut->cols <= 0)) {
		polar_err("lut mismatch [%s]\n", __func__);
		return -1;
	}

	interpolate_find_pos(lut->percent, lut->rows, pc, &row1, &row2);

	if (batt_temp_degc > lut->temp[0])
		batt_temp_degc = lut->temp[0];
	if (batt_temp_degc < lut->temp[lut->cols - 1])
		batt_temp_degc = lut->temp[lut->cols - 1];

	for (i = 0; i < lut->cols; i++)
		if (batt_temp_degc >= lut->temp[i])
			break;
	if ((batt_temp_degc == lut->temp[i]) || !i) {
		ocv = polar_linear_interpolate(lut->ocv[row1][i],
			lut->percent[row1], lut->ocv[row2][i],
			lut->percent[row2], pc);
		return ocv;
	}

	ocvrow1 = polar_linear_interpolate(lut->ocv[row1][i - 1],
		lut->temp[i - 1], lut->ocv[row1][i],
		lut->temp[i], batt_temp_degc);

	ocvrow2 = polar_linear_interpolate(lut->ocv[row2][i - 1],
		lut->temp[i - 1], lut->ocv[row2][i],
		lut->temp[i], batt_temp_degc);

	ocv = polar_linear_interpolate(ocvrow1, lut->percent[row1], ocvrow2,
		lut->percent[row2], pc);

	return ocv;
}
#endif

int get_polar_vector_value(const struct polar_x_y_z_tbl *lut,
	int batt_temp_degc, int soc, int curr, int t_index)
{
	int x_soc, y_curr, z_temp;

	if (!lut || t_index > POLAR_TIME_ARRAY_NUM || t_index < 0 || curr <= 0)
		return 0;
	z_temp =  interpolate_nearest_x(lut->z_array,
		lut->z_lens, batt_temp_degc);
	x_soc = interpolate_nearest_x(lut->x_array, lut->rows, soc);
	y_curr = interpolate_curr_vector(g_polar_curr_vector_interval,
		POLAR_CURR_ARRAY_VECTOR_NUM, curr);
	return lut->value[z_temp][x_soc][y_curr][t_index];
}

int get_polar_vector_res(struct polar_res_tbl *lut,
	int batt_temp_degc, int soc, int curr)
{
	int soc_index, curr_index, batt_temp_index, res;

	if (!lut || soc <= 0 || curr <= 0)
		return 0;
	batt_temp_index = interpolate_nearest_x(lut->x_array, lut->rows,
		batt_temp_degc);
	soc_index = interpolate_nearest_x(lut->y_array, lut->cols, soc);
	curr_index = interpolate_curr_vector(g_polar_curr_vector_interval,
		POLAR_CURR_ARRAY_VECTOR_NUM, curr);
	res = interpolate_two_dimension(lut, lut->x_array[batt_temp_index],
		lut->y_array[soc_index], g_polar_curr_vector_interval[curr_index]);
	return res;
}

/* Find two valid a values within the current temperature range of for interpolation */
static int interpolate_linear_a(const struct polar_learn_tbl *lut,
	int batt_temp_index, int soc_index)
{
	int i, temp_index, pos_index, neg_index, result;
	long pos_trained_a = 0;
	long neg_trained_a = 0;

	temp_index = batt_temp_index;
	pos_index = batt_temp_index;
	neg_index = batt_temp_index;
	for (i = 0; i < POLAR_LEARN_TEMP_RANGE; i++) {
		temp_index++;
		polar_info("%s:vol:%d, temp_index:%d\n", __func__,
			lut->value[soc_index][temp_index].polar_vol_mv, temp_index);
		if ((temp_index >= POLAR_LEARN_TEMP_COLS) || (temp_index < 0))
			break;
		if (lut->value[soc_index][temp_index].polar_vol_mv) {
			pos_trained_a = lut->value[soc_index][temp_index].a_trained;
			pos_index = temp_index;
			break;
		}
	}
	temp_index = batt_temp_index;
	for (i = 0; i < POLAR_LEARN_TEMP_RANGE; i++) {
		temp_index--;
		if ((temp_index >= POLAR_LEARN_TEMP_COLS) || (temp_index < 0))
			break;
		/* The temperature is discontinuous after. When the temperature exceeds, no more valid value is found */
		if (lut->y_array[temp_index] >= TEMP_25_DEGREE)
			break;
		polar_info("%s:vol:%d, temp_index:%d\n", __func__,
			lut->value[soc_index][temp_index].polar_vol_mv, temp_index);
		if (lut->value[soc_index][temp_index].polar_vol_mv) {
			neg_trained_a = lut->value[soc_index][temp_index].a_trained;
			neg_index = temp_index;
			break;
		}
	}
	polar_info("%s:neg_trained_a:%ld, pos_trained_a:%ld\n", __func__,
		neg_trained_a, pos_trained_a);
	if (neg_trained_a && pos_trained_a) {
		result = polar_linear_interpolate(neg_trained_a,
			lut->y_array[neg_index], pos_trained_a,
			lut->y_array[pos_index], lut->y_array[batt_temp_index]);
		return result;
	}
	if (neg_trained_a)
		return neg_trained_a;

	if (pos_trained_a)
		return pos_trained_a;

	return -1;
}

/* used to record the polarization voltage twice (updated every 5s) */
static void record_polar_vol(long polar_vol_uv)
{
	struct hisi_polar_device *di = g_polar_di;
	int i;

	if (!di)
		return;
	mutex_lock(&di->polar_vol_lock);
	for (i = 0; i < POLAR_ARRAY_NUM; i++)
		polar_debug("%s:vol:%ld,vol_now:%ld\n", __func__,
			di->polar_vol_array[i], polar_vol_uv);

	di->polar_vol_array[di->polar_vol_index] = polar_vol_uv;
	di->polar_vol_index++;
	di->polar_vol_index = di->polar_vol_index % POLAR_ARRAY_NUM;
	mutex_unlock(&di->polar_vol_lock);
}

static int polar_flash_open(int flags)
{
	char p_name[POLAR_BUF_SHOW_LEN + 1] = {0};
	int ret, fd_dfx;

	ret = flash_find_ptn_s(PART_BATT_TP_PARA, p_name, sizeof(p_name));
	if (ret)
		polar_err("%s()-line=%d\n", __func__, __LINE__);

	p_name[POLAR_BUF_SHOW_LEN] = '\0';
	fd_dfx = sys_open(p_name, flags, POLAR_FILE_LIMIT);
	if (fd_dfx < 0)
		polar_err("%s()-line=%d\n", __func__, __LINE__);

	return fd_dfx;
}

static u32 polar_get_flash_data(void *buf, u32 buf_size, u32 flash_offset)
{
	int ret, fd_flash;
	u32 cnt = 0;
	mm_segment_t old_fs;

	if (!buf || buf_size == 0) {
		polar_err("%s()-line=%d\n", __func__, __LINE__);
		return 0;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd_flash = polar_flash_open(O_RDONLY);
	if (fd_flash >= 0) {
		ret = sys_lseek(fd_flash, flash_offset, SEEK_SET);
		if (ret < 0) {
			polar_err("%s()-line=%d, ret=%d, flash_offset=%x\n",
				__func__, __LINE__, ret, flash_offset);
			goto close;
		}
		cnt = (u32)sys_read(fd_flash, buf, buf_size);
		if (cnt != buf_size) {
			polar_err("%s()-line=%d, cnt=%u\n",
				__func__, __LINE__, cnt);
			goto close;
		}
	} else {
		polar_err("%s()-line=%d, fd_flash=%d\n",
			__func__, __LINE__, fd_flash);
		set_fs(old_fs);
		return 0;
	}
close:
	sys_close(fd_flash);
	set_fs(old_fs);
	return cnt;
}

void polar_add_flash_data(const void *p_buf, u32 buf_size, u32 flash_offset)
{
	int ret, fd_flash, cnt;
	mm_segment_t old_fs;

	if (!p_buf) {
		polar_err("%s()-line=%d\n", __func__, __LINE__);
		return;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd_flash = polar_flash_open(O_WRONLY);
	if (fd_flash >= 0) {
		ret = sys_lseek(fd_flash, flash_offset, SEEK_SET);
		if (ret < 0) {
			polar_err("%s()-line=%d, ret=%d\n",
				__func__, __LINE__, ret);
			goto close;
		}
		cnt = sys_write(fd_flash, p_buf, buf_size);
		if (cnt != buf_size) {
			polar_err("%s()-line=%d, cnt=%d\n",
				__func__, __LINE__, cnt);
			goto close;
		}
	} else {
		polar_err("%s()-line=%d\n", __func__, __LINE__);
		set_fs(old_fs);
		return;
	}
close:
	sys_close(fd_flash);
	set_fs(old_fs);
}

void polar_clear_flash_data(void)
{
	void *p_buf = NULL;

	p_buf = kzalloc(POLAR_LUT_SIZE, GFP_KERNEL);
	if (!p_buf)
		return;

	pr_err("%s()-line=%d\n", __func__, __LINE__);
	polar_add_flash_data(p_buf, POLAR_LUT_SIZE, POLAR_LUT_OFFSET);
	kfree(p_buf);
}
EXPORT_SYMBOL(polar_clear_flash_data);

static int polar_partition_ready(void)
{
	char p_name[POLAR_BUF_SHOW_LEN + 1] = {0};
	int ret;

	ret = flash_find_ptn_s(PART_BATT_TP_PARA, p_name, sizeof(p_name));
	if (ret)
		polar_err("%s()-line=%d\n", __func__, __LINE__);

	p_name[POLAR_BUF_SHOW_LEN] = '\0';
	if (sys_access(p_name, 0)) {
		polar_err("%s()-line=%d, TP partition name:%s\n",
			__func__, __LINE__, p_name);
		return -1;
	}
	return 0;
}

static int polar_trained_check(const struct polar_learn_tbl *lut,
	int i, int j)
{
	if (lut->value[i][j].polar_vol_mv) {
		if ((lut->value[i][j].a_trained < POLAR_ERR_A_MIN) ||
			(lut->value[i][j].a_trained > POLAR_ERR_A_MAX)) {
			polar_err("fail data lut[%d][%d].vol:%d,.a:%d",
				i, j, lut->value[i][j].polar_vol_mv,
				lut->value[i][j].a_trained);
			return -1;
		}
	}
	return 0;
}

static int polar_partition_data_check(const struct polar_learn_tbl *lut)
{
	int i, j, ret;

	if (!lut)
		return -1;
	for (i = 0; i < lut->rows; i++) {
		for (j = 0; j < lut->cols; j++) {
			ret = polar_trained_check(lut, i, j);
			if (ret)
				return -1;
			polar_debug("self_learn_lut[%d][%d].vol:%d,.a:%d",
				i, j, lut->value[i][j].polar_vol_mv,
				lut->value[i][j].a_trained);
		}
	}
	return 0;
}

#ifdef CONFIG_HISI_DEBUG_FS
ssize_t polar_self_learn_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	void *p_buf = NULL;
	u32 cnt;

	p_buf = kzalloc(POLAR_LUT_SIZE, GFP_KERNEL);
	if (!p_buf)
		return 0;

	/* copy data from emmc, to save in user buffer */
	cnt = polar_get_flash_data(p_buf, POLAR_LUT_SIZE, POLAR_LUT_OFFSET);
	if (cnt > 0) {
		if (memcpy_s((void *)buf, PAGE_SIZE, (const void *)p_buf,
			min_t(size_t, POLAR_LUT_SIZE, PAGE_SIZE)))
			polar_err("%s()-line=%d\n", __func__, __LINE__);
	} else {
		polar_err("%s()-line=%d\n", __func__, __LINE__);
	}
	kfree(p_buf);

	return (ssize_t)cnt;
}
#endif

/* Record the learned a value. */
void store_trained_a(struct polar_learn_tbl *lut,
	int batt_temp_degc, int soc, long trained_a, long polar_vol_uv)
{
	int soc_index, batt_temp_index;
	static unsigned long last_self_learn_time;
	unsigned long time_now =
		(unsigned long)(hisi_getcurtime() / NSEC_PER_SEC);
	unsigned long self_learn_gap;

	if (!lut)
		return;
	soc_index = interpolate_nearest_x(lut->x_array, lut->rows, soc);
	batt_temp_index = interpolate_nearest_x(lut->y_array, lut->cols,
						batt_temp_degc);
	lut->value[soc_index][batt_temp_index].polar_vol_mv =
		(short)(polar_vol_uv / UVOLT_PER_MVOLT);
	lut->value[soc_index][batt_temp_index].a_trained =
		(short)trained_a;

	if (batt_temp_degc >= TEMP_POLAR_REPORT)
		self_learn_gap = SELF_LEARN_GAP_NORMAL;
	else
		self_learn_gap = SELF_LEARN_GAP_LOW;
	if (time_after(time_now, last_self_learn_time + self_learn_gap)) {
		polar_info("%s:last_learn_time:%lu,time_now:%lu\n", __func__,
			last_self_learn_time, time_now);
		last_self_learn_time = time_now;
		polar_add_flash_data(lut->value,
			sizeof(lut->value), POLAR_LUT_OFFSET);
	}
	polar_info("%s:polar trained a:%d, polar vol:%d\n", __func__,
		lut->value[soc_index][batt_temp_index].a_trained,
		lut->value[soc_index][batt_temp_index].polar_vol_mv);
}

/* To query the polarization voltage corresponding to the a value in the current auto-learning */
short get_trained_polar_vol(const struct polar_learn_tbl *lut,
	int batt_temp_degc, int soc)
{
	int soc_index, batt_temp_index;

	if (!lut)
		return 0;
	soc_index = interpolate_nearest_x(lut->x_array, lut->rows, soc);
	batt_temp_index = interpolate_nearest_x(lut->y_array, lut->cols,
		batt_temp_degc);
	return lut->value[soc_index][batt_temp_index].polar_vol_mv;
}

/* To query the a value of the auto-learning */
long get_trained_a(const struct polar_learn_tbl *lut,
	int batt_temp_degc, int soc)
{
	int soc_index, batt_temp_index;
	long trained_a = -1;

	if (!lut)
		return -1;
	soc_index = interpolate_nearest_x(lut->x_array, lut->rows, soc);
	batt_temp_index = interpolate_nearest_x(lut->y_array, lut->cols,
		batt_temp_degc);
	polar_info("%s:temp:%d,temp_idx:%d,soc:%d,soc_idx:%d\n", __func__,
		   batt_temp_degc, batt_temp_index, soc, soc_index);

	if ((batt_temp_index >= POLAR_LEARN_TEMP_COLS) || (batt_temp_index  < 0))
		return -1;
	if ((soc_index >= POLAR_RES_PC_CURR_ROWS) || (soc_index < 0))
		return -1;
	/*
	 * If the polarization voltage in the self-learning table
	 * corresponding to the current battery temperature is not 0,
	 * the value of a is valid.
	 */
	if (lut->value[soc_index][batt_temp_index].polar_vol_mv)
		trained_a = lut->value[soc_index][batt_temp_index].a_trained;
	else if (batt_temp_degc < TEMP_25_DEGREE)
		/*
		 * If the current temperature is less than and
		 * the temperature and power nodes do not have self-learned values,
		 * obtain the a value through interpolation.
		 */
		trained_a = interpolate_linear_a(lut, batt_temp_index, soc_index);
	return trained_a;
}

/* Querying Whether Battery Conditions Can Be Self-learned */
bool could_vbat_learn_a(struct hisi_polar_device *di,
	struct polar_com_para *para, int vol_now_mv, long polar_vol_uv)
{
	int vol_coe, delta_vol;
	long polar_vol_trained;
	long polar_vol_mv = polar_vol_uv / UVOLT_PER_MVOLT;

	if (!di)
		return FALSE;
	polar_debug("%s:v_cutoff:%u\n", __func__, di->v_cutoff);

	if (VBAT_LEARN_GAP_MV <= (int)di->v_cutoff - vol_now_mv)
		return FALSE;

	delta_vol = para->ocv_soc_mv - (int)di->v_cutoff +
		(para->cur * ((int)di->r_pcb / UOHM_PER_MOHM)) / UVOLT_PER_MVOLT;
	if (delta_vol == 0)
		return FALSE;

	vol_coe = (TENTH * (para->ocv_soc_mv - vol_now_mv)) / delta_vol;
	polar_debug("%s:vol_coe:%d, last_avgcurr_5s:%d\n", __func__,
		vol_coe, di->last_avgcurr_5s);
	if (vol_coe > VBAT_LEARN_COE_HIGH)
		return TRUE;

	if (di->last_avgcurr_5s < VBAT_LEARN_AVGCURR_HIGH)
		return TRUE;
	polar_vol_trained = get_trained_polar_vol(
		&g_polar_learn_lut, para->temp, para->soc);
	polar_info("%s:trained_vol:%ld\n", __func__, polar_vol_trained);

	if ((vol_coe > VBAT_LEARN_COE_LOW) && (polar_vol_mv < polar_vol_trained))
		return TRUE;

	if ((di->last_avgcurr_5s < VBAT_LEARN_AVGCURR_LOW) &&
		(polar_vol_mv < polar_vol_trained))
		return TRUE;
	/* If the preceding conditions are not met, self-learning is not performed. */
	return FALSE;
}

/* Querying Whether Battery Conditions Can Be Self-learned  */
static bool could_learn_a(struct hisi_polar_device *di,
	struct polar_com_para *para, int vol_now_mv, long polar_vol_uv)
{
	long last_polar_vol = 0;

	if (!di)
		return FALSE;
	/* Vpert(tn-1)pert(tn)<0 discharge current is negative, and Vpert(tn)<Vpert(tn-1) */
	if (polar_vol_uv >= 0)
		return FALSE;
	mutex_lock(&di->polar_vol_lock);
	if ((di->polar_vol_index >= 0) && (di->polar_vol_index < POLAR_ARRAY_NUM))
		last_polar_vol = (di->polar_vol_index - 1 < 0) ?
			di->polar_vol_array[POLAR_ARRAY_NUM - 1] :
			di->polar_vol_array[di->polar_vol_index - 1];

	polar_debug("%s:last_polar_vol:%ld,polar_vol_now:%ld\n", __func__,
		last_polar_vol, polar_vol_uv);
	if ((last_polar_vol >= 0) || ((last_polar_vol <= polar_vol_uv) &&
		((vol_now_mv - (int)di->v_cutoff) >= VBAT_LEARN_NOLIMIT_MV))) {
		mutex_unlock(&di->polar_vol_lock);
		return FALSE;
	}
	mutex_unlock(&di->polar_vol_lock);
	/* Determine other battery-related conditions. */
	polar_debug("%s:ocv:%d,vol_now:%d,cur:%d,temp:%d,soc:%d\n", __func__,
		para->ocv_soc_mv, vol_now_mv, para->cur, para->temp, para->soc);
	if (!could_vbat_learn_a(di, para, vol_now_mv, polar_vol_uv))
		return FALSE;
	return TRUE;
}

static bool could_update_b(struct hisi_polar_device *di, long polar_vol_uv)
{
	int i;

	if (!di)
		return FALSE;
	/* judge if can update polar_b with current polar voltage */
	if ((polar_vol_uv < VPERT_NOW_LOW_B) || (polar_vol_uv > VPERT_NOW_HIGH_B))
		return FALSE;
	mutex_lock(&di->polar_vol_lock);
	/* judge if can update polar_b with former two polar voltages */
	for (i = 0; i < POLAR_ARRAY_NUM; i++) {
		if (di->polar_vol_array[i] == POLAR_VOL_INVALID)
			break;
		if ((di->polar_vol_array[i] < VPERT_PAST_LOW_B) ||
			di->polar_vol_array[i] > VPERT_PAST_HIGH_B)
			break;
	}
	if (i == POLAR_ARRAY_NUM) {
		mutex_unlock(&di->polar_vol_lock);
		return FALSE;
	}
	mutex_unlock(&di->polar_vol_lock);
	return TRUE;
}

static void update_polar_error_b(int ocv_soc_mv, int vol_now_mv,
	long polar_vol_uv)
{
	struct hisi_polar_device *di = g_polar_di;

	if (!di)
		return;
	if (!could_update_b(di, polar_vol_uv))
		return;
	/* calculate polar b */
	g_polar_err_b1 = (long)(ocv_soc_mv - vol_now_mv) * UVOLT_PER_MVOLT;
	g_polar_err_b2 = polar_vol_uv;
	polar_debug("update polar b1,b2:%ld:%ld\n", g_polar_err_b1, g_polar_err_b2);
}

static long calc_weighted_a(long a_trained, int temp, int enqueue)
{
	int i;
	long temp_err_a_wavg = 0;
	static int polar_valid_a_index;
	static int last_a_idx;
	int temp_abs;
	long a_weighted;

	/* not the first time with valid a */
	if ((last_a_idx != polar_valid_a_index) && enqueue) {
		temp_abs = g_polar_err_a_array[last_a_idx].temp - temp;
		temp_abs = abs(temp_abs);
		/* when temp change more than 3 degree */
		if (temp_abs >= 3) {
			for (i = 0; i < POLAR_VALID_A_NUM; i++) {
				g_polar_err_a_array[i].err_a = a_trained;
				g_polar_err_a_array[i].temp = temp;
			}
			polar_info("%s:temp_abs:%d, trained_a:%ld\n", __func__,
				temp_abs, a_trained);
		}
	}
	/* calculate weighted average of A with former three A values */
	for (i = 0; i < POLAR_VALID_A_NUM; i++) {
		polar_debug("%s:a_array[%d]:%ld,coe:%d\n", __func__, i,
			g_polar_err_a_array[i].err_a, g_polar_err_a_coe[i]);
		if (!g_polar_err_a_array[i].err_a)
			temp_err_a_wavg += (a_trained * g_polar_err_a_coe[i]);
		else
			temp_err_a_wavg += (g_polar_err_a_array[i].err_a *
				g_polar_err_a_coe[i]);
	}
	a_weighted = (a_trained * g_polar_err_a_coe[POLAR_VALID_A_NUM] +
		temp_err_a_wavg) / POLAR_A_COE_MUL;
	if (enqueue) {
		g_polar_err_a_array[polar_valid_a_index % POLAR_VALID_A_NUM]
			.err_a = a_trained;
		g_polar_err_a_array[polar_valid_a_index % POLAR_VALID_A_NUM]
			.temp = temp;
		last_a_idx = polar_valid_a_index;
		polar_valid_a_index++;
		polar_valid_a_index = polar_valid_a_index % POLAR_VALID_A_NUM;
	}
	polar_info("%s:update real polar a:%ld, weighted average a:%ld\n",
		__func__, a_trained, a_weighted);
	return a_weighted;
}

static void polar_learn_lut_init(struct polar_learn_tbl *lut)
{
	int i, j;

	if (!lut)
		return;
	for (i = 0; i < lut->rows; i++) {
		for (j = 0; j < lut->cols; j++) {
			lut->value[i][j].a_trained = POLAR_ERR_A_DEFAULT;
			lut->value[i][j].polar_vol_mv = 0;
		}
	}
}

static void update_polar_error_a(struct polar_com_para *para, int vol_now_mv,
	long polar_vol_uv)
{
	long temp_err_a, a_trained;
	static int polar_partition_read_flag;
	struct hisi_polar_device *di = g_polar_di;

	if (!di)
		return;
	if (polar_partition_ready()) {
		g_polar_err_a = -1;
		return;
	}
	/* read polar partition once partition is ready */
	if (!polar_partition_read_flag) {
		polar_get_flash_data(g_polar_learn_lut.value,
			sizeof(g_polar_learn_lut.value), POLAR_LUT_OFFSET);
		if (polar_partition_data_check(&g_polar_learn_lut)) {
			polar_clear_flash_data();
			polar_learn_lut_init(&g_polar_learn_lut);
		}
		polar_partition_read_flag = 1;
	}
	if ((polar_vol_uv >= VPERT_NOW_LOW_A) && (polar_vol_uv <= VPERT_NOW_HIGH_A)) {
		temp_err_a = POLAR_ERR_A_DEFAULT;
		goto get_a;
	}
	temp_err_a = ((long)(para->ocv_soc_mv  - vol_now_mv) * UVOLT_PER_MVOLT -
		g_polar_err_b1) * POLAR_ERR_COE_MUL / (-polar_vol_uv);
	/* if polar_a was negative,we use last max average current instead */
	if (temp_err_a <= 0) {
		temp_err_a = POLAR_ERR_A_DEFAULT;
		goto get_a;
	}
	/* clamp a in range[0.9~3] */
	temp_err_a = clamp_val(temp_err_a, POLAR_ERR_A_MIN, POLAR_ERR_A_MAX);

	/* check whether update in self learning */
	if (could_learn_a(di, para, vol_now_mv, polar_vol_uv))
		store_trained_a(&g_polar_learn_lut,
			para->temp, para->soc, temp_err_a, polar_vol_uv);
get_a:
	a_trained = get_trained_a(&g_polar_learn_lut, para->temp, para->soc);
	polar_info("%s:polar a before weighted average:%ld, trained_a:%ld\n",
		__func__, temp_err_a, a_trained);
	if (a_trained <= 0) {
		polar_info("no self learn polar a ,use current value\n");
		g_polar_err_a = calc_weighted_a(temp_err_a, para->temp, 0);
	} else {
		g_polar_err_a = calc_weighted_a(a_trained, para->temp, 1);
	}
}

static int calc_res_future(int curr_tmp, int batt_temp_degc, int soc, int r_pcb)
{
	int polar_res_future;
	int polar_vector = 0;
	int t_index;
	int res, res_zero, res_vector;

	res = interpolate_two_dimension(&g_polar_res_lut1, batt_temp_degc, soc,
		curr_tmp);
	res_vector = get_polar_vector_res(&g_polar_res_lut1, batt_temp_degc, soc,
		curr_tmp);
	if (res_vector == 0)
		return 0;
	for (t_index = 0; t_index < POLAR_VECTOR_5S; t_index++) {
		res_zero = get_polar_vector_value(&g_polar_vector_lut,
			batt_temp_degc, soc, curr_tmp, t_index + 1);
		polar_vector += res_zero;
		polar_debug("res_vector[%d]:%d", t_index + 1, res_zero);
	}
	res_zero = get_polar_vector_value(&g_polar_vector_lut, batt_temp_degc,
		soc, curr_tmp, 0);
	polar_res_future = ((res_zero + polar_vector) * res) / res_vector;
	/* calculate polar_res_future in */
	polar_res_future = (polar_res_future * g_polar_err_a) /
		(POLAR_ERR_COE_MUL * POLAR_RES_MHOM_MUL) +
		r_pcb / UOHM_PER_MOHM;
	polar_debug("%s:polar_res_future:%d, err_a:%ld, r_pcb:%d\n", __func__,
		polar_res_future, g_polar_err_a, r_pcb);
	return polar_res_future;
}

static int select_predict_fifo(int *fifo, int size)
{
	int i;

	if (size <= 1 || size > (MAX_PREDICT_CYCLE + 1))
		return CURR_AVG_DEFAULT;

	if (fifo[0] > fifo[1]) {
		for (i = 1; i < (size - 1); i++) {
			if (fifo[i] < fifo[i + 1])
				break;
		}
	} else {
		for (i = 1; i < (size - 1); i++) {
			if (fifo[i] > fifo[i + 1])
				break;
		}
	}
	return fifo[i];
}

static int predict_avg_curr(long polar_vol_future, int curr_tmp,
	int batt_temp_degc, int soc, int r_pcb)
{
	int i;
	int polar_res_future;
	int curr_future;
	int curr_fifo[MAX_PREDICT_CYCLE + 1] = {0};
	struct hisi_polar_device *di = g_polar_di;

	if (!di)
		return 0;
	curr_fifo[0] = curr_tmp;
	for (i = 0; i < MAX_PREDICT_CYCLE; i++) {
		polar_res_future =
			calc_res_future(curr_tmp, batt_temp_degc, soc, r_pcb);
		if (polar_res_future != 0)
			curr_tmp = polar_vol_future / polar_res_future;
		else
			polar_err("%s:res wrong, use last value\n", __func__);
		curr_fifo[i + 1] = curr_tmp;
		polar_debug("%s:predict:curr:%d,polar_res_future:%d\n",
			__func__, curr_tmp, polar_res_future);
	}
	curr_future = select_predict_fifo(
		curr_fifo, MAX_PREDICT_CYCLE + 1);
	di->polar_res_future = polar_res_future;
	di->last_max_avg_curr = curr_future;
	return curr_future;
}

/* Calculate the maximum load current */
static int get_estimate_max_avg_curr(const struct polar_com_para *para,
	long polar_past, int v_cutoff, int r_pcb)
{
	int curr_index;
	long polar_vol_future;
	int polar_res_future;
	int curr_tmp = 0;
	int curr_thresh;
	struct hisi_polar_device *di = g_polar_di;

	if (!di || !para)
		return 0;

	if (g_polar_err_a < 0)
		return 0;
	/* calculate polar_vol_future in uV */
	polar_vol_future = (long)(para->ocv_soc_mv - v_cutoff) *
		UVOLT_PER_MVOLT - (g_polar_err_b1 +
		g_polar_err_a * g_polar_err_b2 / POLAR_ERR_COE_MUL) +
		(polar_past * g_polar_err_a / POLAR_ERR_COE_MUL);
	polar_debug("%s:polar_vol_future:%ld,ocv_soc_mv:%d,v_cutoff:%d,polar_err_b1:%ld,polar_err_b2:%ld,polar_err_a:%ld,polar_past:%ld\n",
			__func__, polar_vol_future, para->ocv_soc_mv, v_cutoff,
			g_polar_err_b1, g_polar_err_b2, g_polar_err_a, polar_past);
	for (curr_index = POLAR_CURR_ARRAY_VECTOR_NUM - 1; curr_index >= 0;
		curr_index--) {
		polar_res_future =
			calc_res_future(g_polar_curr_vector_interval[curr_index],
				para->temp, para->soc, r_pcb);
		if (polar_res_future != 0)
			curr_tmp = polar_vol_future / polar_res_future;
		polar_debug("%s:polar_vol_future:%ld,polar_res_future:%d\n",
			__func__, polar_vol_future, polar_res_future);
		if (curr_index > 0)
			curr_thresh = two_avg(
				g_polar_curr_vector_interval[curr_index],
				g_polar_curr_vector_interval[curr_index - 1]);
		else
			curr_thresh = g_polar_curr_vector_interval[0];
		if (curr_thresh <= curr_tmp) {
			curr_tmp = predict_avg_curr(polar_vol_future, curr_tmp,
				para->temp, para->soc, r_pcb);
			return curr_tmp;
		}
	}
	di->polar_res_future = polar_res_future;
	di->last_max_avg_curr = curr_tmp;
	return curr_tmp;
}

static int calc_r0_future(int curr_tmp, int batt_temp_degc, int soc)
{
	int res_zero, res, res_vector;

	res = interpolate_two_dimension(&g_polar_res_lut0, batt_temp_degc, soc,
		curr_tmp);
	res_vector = get_polar_vector_res(&g_polar_res_lut0, batt_temp_degc, soc,
		curr_tmp);
	res_zero = get_polar_vector_value(&g_polar_vector_lut, batt_temp_degc,
		soc, curr_tmp, 0);
	if (res_vector)
		res_zero = (res_zero * res) / res_vector;
	return res_zero;
}

static int predict_peak_curr(long polar_vol_future, int curr_tmp,
	int batt_temp_degc, int soc, int r_pcb)
{
	int i;
	int polar_r0_future;
	int curr_future;
	int r0_temp;
	int curr_fifo[MAX_PREDICT_CYCLE + 1] = {0};

	curr_fifo[0] = curr_tmp;
	for (i = 0; i < MAX_PREDICT_CYCLE; i++) {
		polar_r0_future = calc_r0_future(curr_tmp, batt_temp_degc, soc);
		r0_temp = r_pcb / UOHM_PER_MOHM +
			g_polar_err_a * polar_r0_future /
			(POLAR_RES_MHOM_MUL * POLAR_ERR_COE_MUL);
		/* voltage in uv,resistence in curr in mA */
		if (r0_temp != 0)
			curr_tmp = (polar_vol_future * UVOLT_PER_MVOLT -
				(g_polar_err_b1 + g_polar_err_a * g_polar_err_b2 /
				 POLAR_ERR_COE_MUL)) / r0_temp;
		else
			polar_err("%s:res wrong, use last value", __func__);
		curr_fifo[i + 1] = curr_tmp;
		polar_debug("%s:predict:curr_future:%d,polar_r0_future:%d, polar_vol_future:%ld\n",
			__func__, curr_tmp, polar_r0_future, polar_vol_future);
	}
	curr_future = select_predict_fifo(curr_fifo, MAX_PREDICT_CYCLE + 1);
	return curr_future;
}

static int get_estimate_peak_curr(int ocv_soc_mv, int soc,
	int batt_temp_degc, int v_cutoff, int r_pcb)
{
	int curr_index, res_zero, curr_future, curr_temp;
	int curr_thresh;
	struct hisi_polar_device *di = g_polar_di;

	if (!di)
		return 0;

	if (g_polar_err_a < 0)
		return 0;

	for (curr_index = POLAR_CURR_ARRAY_VECTOR_NUM - 1;
		curr_index >= 0; curr_index--) {
		curr_future = g_polar_curr_vector_interval[curr_index];
		res_zero = calc_r0_future(curr_future, batt_temp_degc, soc);
		/* voltage in uv,resistence in,curr in mA */
		curr_temp =
			((long)(ocv_soc_mv - v_cutoff) * UVOLT_PER_MVOLT -
			(g_polar_err_b1 + g_polar_err_a *
				g_polar_err_b2 / POLAR_ERR_COE_MUL)) /
			(r_pcb / UOHM_PER_MOHM + g_polar_err_a * res_zero /
			(POLAR_RES_MHOM_MUL * POLAR_ERR_COE_MUL));
		polar_debug("%s:curr_future:%d, res_zero:%d, curr_temp:%d, ocv_soc_mv:%d, v_cutoff:%d\n",
					__func__, curr_future, res_zero,
					curr_temp, ocv_soc_mv, v_cutoff);
		if (curr_index > 0)
			curr_thresh = two_avg(
				g_polar_curr_vector_interval[curr_index],
				g_polar_curr_vector_interval[curr_index - 1]);
		else
			curr_thresh = g_polar_curr_vector_interval[0];
		if (curr_thresh <= curr_temp) {
			curr_future = predict_peak_curr(
				(ocv_soc_mv - v_cutoff), curr_temp,
				batt_temp_degc, soc, r_pcb);
			di->last_max_peak_curr = curr_temp;
			return curr_temp;
		}
	}
	di->last_max_peak_curr = curr_temp;
	return curr_temp;
}

static int calc_per_duration_polar_voltage(struct polar_res_tbl *res_lut,
	int t_index, int curr_index, int temp)
{
	int soc_avg, curr_avg, ratio;
	int res, res_vector, polar_vector;
	int vol = 0;

	soc_avg = g_polar_avg_curr_info[t_index][0].soc_avg;
	curr_avg = g_polar_avg_curr_info[t_index][curr_index].current_avg;
	ratio = g_polar_avg_curr_info[t_index][curr_index].duration;
	if ((ratio == 0) || (soc_avg == 0) || (curr_avg == 0))
		return vol;
	res = interpolate_two_dimension(res_lut,
		temp, soc_avg, abs(curr_avg));
	res_vector = get_polar_vector_res(res_lut,
		temp, soc_avg, abs(curr_avg));
	polar_vector = get_polar_vector_value(&g_polar_vector_lut,
		temp, soc_avg, abs(curr_avg), t_index + 1);
	if (res_vector)
		vol = (long)curr_avg * ((((long)ratio * (res / POLAR_RES_MHOM_MUL)) /
			POLAR_RATIO_PERCENTAGE) * polar_vector) / res_vector;
	return vol;
}

static long calculate_polar_volatge(
	struct polar_res_tbl *res_lut, int soc, int temp)
{
	int t_index;
	int curr_index;

	long vol_sum = 0;
	struct hisi_polar_device *di = g_polar_di;

	if (!di || !res_lut)
		return 0;
	for (t_index = 0; t_index < POLAR_TIME_ARRAY_NUM; t_index++) {
		for (curr_index = 0; curr_index < (POLAR_CURR_ARRAY_NUM + 1); curr_index++) {
			vol_sum += calc_per_duration_polar_voltage(res_lut,
				t_index, curr_index, temp);
			polar_debug("vol_sum:%ld\n", vol_sum);
		}
	}
	return vol_sum;
}

static long calculate_polar_vol_r0(int soc, int temp, int curr)
{
	int res_zero;
	long polar_vol;

	res_zero = calc_r0_future(abs(curr), temp, soc);
	polar_vol = (long)curr * res_zero / POLAR_RES_MHOM_MUL;
	polar_debug("res_zero:%d,vol0:%ld\n", res_zero, polar_vol);
	return polar_vol;
}

static void add_polar_info(int current_ma, int duration, int t_index)
{
	int curr_index;

	if ((t_index < 0) ||
		(t_index >= POLAR_TIME_ARRAY_NUM) || (duration <= 0))
		return;
	/* find the corresponding current range. */
	for (curr_index = 0; curr_index < POLAR_CURR_ARRAY_NUM; curr_index++) {
		if (abs(current_ma) <= g_polar_curr_interval[curr_index])
			break;
	}
	/*
	 * Whether reverse polarization needs to be considered is not specified in the solution.
	 * The current is replaced by an absolute value.
	 * If the current is greater than 2c,
	 * the value of POLAR_CURR_ARRAY_NUM needs to be calculated.
	 */
	g_polar_avg_curr_info[t_index][curr_index].current_avg +=
		((long)current_ma * duration);
	g_polar_avg_curr_info[t_index][curr_index].cnt++;
	g_polar_avg_curr_info[t_index][curr_index].duration += duration;
}

static void calc_average_curr(int t_index, int curr_index)
{
	int time_interval;

	if (g_polar_avg_curr_info[t_index][curr_index].duration) {
		g_polar_avg_curr_info[t_index][curr_index].current_avg /=
			g_polar_avg_curr_info[t_index][curr_index].duration;
		/* calculate time ratio corresponding to the current */
		if (t_index)
			time_interval = (int)(g_polar_sample_interval[t_index] -
				g_polar_sample_interval[t_index - 1]);
		else
			time_interval = (int)g_polar_sample_interval[t_index];
		g_polar_avg_curr_info[t_index][curr_index].duration =
			(g_polar_avg_curr_info[t_index][curr_index].duration *
			POLAR_RATIO_PERCENTAGE) / time_interval;
		if (g_polar_avg_curr_info[t_index][curr_index].duration >
			POLAR_RATIO_PERCENTAGE)
			g_polar_avg_curr_info[t_index][curr_index].duration =
				POLAR_RATIO_PERCENTAGE;
	}
}

static void update_polar_avg_curr(struct hisi_polar_device *di,
	int predict_msec)
{
	int t_index;
	int curr_index;
	long past_avg_cc = 0;

	if (!predict_msec) {
		for (t_index = 0; t_index < POLAR_TIME_ARRAY_NUM; t_index++) {
			for (curr_index = 0;
				curr_index < (POLAR_CURR_ARRAY_NUM + 1);
				curr_index++)
				past_avg_cc +=
					g_polar_avg_curr_info[t_index][curr_index].current_avg;

			if (t_index == POLAR_TIME_ARRAY_0S)
				di->last_avgcurr_0s =
					(int)(past_avg_cc / POLAR_TIME_0S);

			if (t_index == POLAR_TIME_ARRAY_5S)
				di->last_avgcurr_5s =
					(int)(past_avg_cc / POLAR_TIME_5S);
			if (t_index == POLAR_TIME_ARRAY_25S) {
				di->last_avgcurr_25s =
					(int)(past_avg_cc / POLAR_TIME_25S);
				break;
			}
		}
		polar_debug("last_avgcurr_0s:%d,5s:%d,25s:%d\n",
			di->last_avgcurr_0s, di->last_avgcurr_5s,
			di->last_avgcurr_25s);
	}

	/* average current in the Tn range */
	for (t_index = 0; t_index < POLAR_TIME_ARRAY_NUM; t_index++) {
		for (curr_index = 0; curr_index < (POLAR_CURR_ARRAY_NUM + 1); curr_index++) {
			/* average current calculation cc/t */
			calc_average_curr(t_index, curr_index);
			polar_debug("polar_avg_curr_info[%d][%d]|duration_ratio|soc_avg:%lld ma,%ld%%,%d\n",
				t_index, curr_index,
				g_polar_avg_curr_info[t_index][curr_index].current_avg,
				g_polar_avg_curr_info[t_index][curr_index].duration,
				g_polar_avg_curr_info[t_index][0].soc_avg);
		}
	}
}

static bool judge_tn_sample_end_time(unsigned long *node_sample_time,
	unsigned long *node_duration_time, int t_index,
	unsigned long head_sample_time, struct ploarized_info *ppolar)
{
	unsigned long temp_duration_time;

	/* sample end time of each node is in Tn */
	if (time_after_eq(*node_sample_time + g_polar_sample_interval[t_index],
		head_sample_time)) {
		/* sample start time of each node is in Tn */
		if (time_after_eq(*node_sample_time - *node_duration_time +
			g_polar_sample_interval[t_index], head_sample_time)) {
			add_polar_info(ppolar->current_ma,
				(int)*node_duration_time, t_index);
			return true;
		}
		/* sample start time of each node is not in Tn, divide node */
		temp_duration_time = *node_sample_time -
			(head_sample_time - g_polar_sample_interval[t_index]);
		add_polar_info(ppolar->current_ma, (int)temp_duration_time,
			t_index);
		*node_duration_time = *node_duration_time - temp_duration_time;
		*node_sample_time = head_sample_time -
			g_polar_sample_interval[t_index];
	}
	return false;
}

static void polar_info_calc(struct hisi_polar_device *di, int predict_msec)
{
	int t_index = 0;
	int last_soc_avg;
	struct ploarized_info *ppolar = NULL;
	struct list_head *pos = NULL;
	unsigned long node_sample_time, node_duration_time, head_sample_time;

	if (!di || !di->polar_buffer) {
		polar_err("[polar] %s di is null\n", __func__);
		return;
	}
	if (list_empty(&(di->polar_head.list)))
		return;
	(void)memset_s(g_polar_avg_curr_info, sizeof(g_polar_avg_curr_info),
		0, sizeof(g_polar_avg_curr_info));
	/* get head element */
	ppolar = list_first_entry(&(di->polar_head.list),
		struct ploarized_info, list);
	last_soc_avg = ppolar->soc_now;
	head_sample_time = ppolar->sample_time + predict_msec;
	/* calculate the average current of each current range in the Tn time */
	list_for_each(pos, &(di->polar_head.list)) {
		ppolar = list_entry(pos, struct ploarized_info, list);
		node_sample_time = ppolar->sample_time;
		node_duration_time = ppolar->duration;
		/* big data divide start */
		while (t_index < POLAR_TIME_ARRAY_NUM) {
			if (judge_tn_sample_end_time(&node_sample_time,
				&node_duration_time, t_index,
				head_sample_time, ppolar))
				break;
			/* sample end time of each node is in Tn */
			/* calculate the center value of electricity in the Tn range */
			g_polar_avg_curr_info[t_index][0].soc_avg =
				two_avg(ppolar->soc_now, last_soc_avg);
			last_soc_avg = ppolar->soc_now;
			/*
			 * if the time of a node is longer than the time of the head node,
			 * the node enters the next Tn range.
			 */
			t_index++;
		}
	}
	update_polar_avg_curr(di, predict_msec);
}

static void fill_up_polar_fifo(struct hisi_polar_device *di,
	struct ploarized_info *ppolar, struct list_head *head,
	struct hisiap_ringbuffer_s *rbuffer, unsigned long total_sample_time)
{
	struct ploarized_info *ppolar_head = NULL;
	struct ploarized_info *ppolar_tail = NULL;
	struct ploarized_info *ppolar_buff = NULL;
	u32 buff_pos = 0;

	if (!ppolar || !rbuffer || !head || !di)
		return;

	if (!list_empty(head)) {
		ppolar_tail = list_last_entry(head,
			struct ploarized_info, list);
		ppolar_head = list_first_entry(head,
			struct ploarized_info, list);
		/* judge if the node is before the head sample time */
		if (time_before(ppolar->sample_time,
			ppolar_head->sample_time + di->fifo_interval))
			return;
		/* judge if we need to del node after total_sample_time */
		if (time_after(ppolar->sample_time,
			(ppolar_tail->sample_time + total_sample_time)))
			list_del(&ppolar_tail->list);
	}
	hisiap_ringbuffer_write(rbuffer, (u8 *)ppolar);
	/* this is to get the buff_pos after write */
	if (!rbuffer->rear) {
		if (rbuffer->is_full) {
			buff_pos = rbuffer->max_num - 1;
		} else {
			polar_err("[%s]:ringbuffer write failed\n", __func__);
			return;
		}
	} else {
		buff_pos = rbuffer->rear - 1;
	}
	ppolar_buff = (struct ploarized_info *)&rbuffer->data[
		(unsigned long)buff_pos * rbuffer->field_count];
	list_add(&(ppolar_buff->list), head);
}

static unsigned long polar_get_head_time(struct hisi_polar_device *di,
	struct list_head *head)
{
	struct ploarized_info *ppolar_head = NULL;
	unsigned long sample_time;

	if (!di || !head)
		return 0;
	if (list_empty(head))
		return 0;
	ppolar_head = list_first_entry(head, struct ploarized_info, list);
	sample_time = ppolar_head->sample_time;
	return sample_time;
}

static int polar_get_head_curr(struct hisi_polar_device *di,
	struct list_head *head)
{
	struct ploarized_info *ppolar_head = NULL;
	int sample_curr;

	if (!di || !head)
		return 0;
	if (list_empty(head))
		return 0;
	ppolar_head = list_first_entry(head, struct ploarized_info, list);
	sample_curr = ppolar_head->current_ma;
	return sample_curr;
}

/* coul fifo sample time callback */
static enum hrtimer_restart sample_timer_func(struct hrtimer *timer)
{
	struct hisi_polar_device *di = g_polar_di;
	struct ploarized_info node;
	unsigned long sample_time, flags, fifo_time_ms;
	int fifo_depth, current_ua, i;
	ktime_t kt;

	if (!di)
		return HRTIMER_NORESTART;
	spin_lock_irqsave(&di->coul_fifo_lock, flags);
	/* get coul fifo according to the fifo depth */
	fifo_depth = coul_drv_battery_fifo_depth();
	sample_time = hisi_getcurtime();
	sample_time = sample_time / NSEC_PER_MSEC;
	for (i = fifo_depth - 1; i >= 0; i--) {
		node.sample_time = sample_time - di->fifo_interval * i;
		current_ua = coul_drv_battery_fifo_curr(i);
		node.current_ma = -(current_ua / UA_PER_MA);
		node.duration = di->fifo_interval;
		node.temperature = coul_drv_battery_temperature();
		node.soc_now = coul_drv_battery_ufcapacity_tenth();
		node.list.next = NULL;
		node.list.prev = NULL;
		polar_debug("%s:time:%lu,curr:%d,duration:%lu,temp:%d:soc:%d\n",
			__func__, node.sample_time, node.current_ma,
			node.duration, node.temperature, node.soc_now);
		/* here we put the fifo info to 30S ringbuffer */
		fill_up_polar_fifo(di, &node, &di->coul_fifo_head.list,
			di->fifo_buffer, COUL_FIFO_SAMPLE_TIME);
	}
	fifo_time_ms = di->fifo_interval * fifo_depth;
	kt = ktime_set(fifo_time_ms / MSEC_PER_SEC,
		(fifo_time_ms % MSEC_PER_SEC) * NSEC_PER_MSEC);
	hrtimer_forward_now(timer, kt);
	spin_unlock_irqrestore(&di->coul_fifo_lock, flags);
	return HRTIMER_RESTART;
}

static long get_polar_vol(struct hisi_polar_device *di, int soc_now,
	int batt_temp, int curr_now)
{
	long vol;

	if (!di)
		return 0;
	vol = calculate_polar_volatge(&g_polar_res_lut, soc_now, batt_temp);
	vol += calculate_polar_vol_r0(soc_now, batt_temp, curr_now);
	return vol;
}

/* Copy the 35-second data in the FIFO to the 20-minute buffer */
void copy_fifo_buffer(struct list_head *fifo_head,
	struct list_head *polar_head, struct hisiap_ringbuffer_s *fifo_rbuffer,
	struct hisiap_ringbuffer_s *polar_rbuffer)
{
	struct hisi_polar_device *di = g_polar_di;
	struct ploarized_info *ppolar_temp = NULL;
	struct ploarized_info *n = NULL;

	if (!fifo_head || !polar_head || !di || !fifo_rbuffer || !polar_rbuffer)
		return;
	if (list_empty(fifo_head))
		return;
	list_for_each_entry_safe_reverse(ppolar_temp, n, fifo_head, list) {
		fill_up_polar_fifo(di, ppolar_temp, polar_head, polar_rbuffer,
			COUL_POLAR_SAMPLE_TIME);
	}
}

static unsigned long get_polar_sample_info(int *fifo_depth,
	unsigned long *sync_time, unsigned long *sample_time, int *head_curr)
{
	struct hisi_polar_device *di = g_polar_di;

	if (!di)
		return 0;
	*fifo_depth = coul_drv_battery_fifo_depth();
	*sync_time = hisi_getcurtime();
	*sync_time = *sync_time / NSEC_PER_MSEC;
	*sample_time = polar_get_head_time(di, &di->coul_fifo_head.list);
	*head_curr = polar_get_head_curr(di, &di->coul_fifo_head.list);

	return (*sync_time - *sample_time) % di->fifo_interval;
}

void sync_sample_info(void)
{
	struct hisi_polar_device *di = g_polar_di;
	unsigned long l_info[L_MAX];
	int i_info[I_MAX];
	int sync_num, i;
	struct ploarized_info node;
	ktime_t kt;

	if (!di)
		return;

	spin_lock_irqsave(&di->coul_fifo_lock, l_info[L_FLAG]);
	/* read polar info from the fifo */
	l_info[L_DELTA] = get_polar_sample_info(&i_info[I_FIF0], &l_info[L_SYNC],
		&l_info[L_SAMPLE], &i_info[I_HEAD]);
	if (time_after_eq(l_info[L_SYNC],
		l_info[L_SAMPLE] + di->fifo_interval)) {
		sync_num = (int)(l_info[L_SYNC] -
			l_info[L_SAMPLE]) / (int)di->fifo_interval;
		/* check whether add one more fifo when we have left time after divide */
		if (l_info[L_DELTA] && sync_num < i_info[I_FIF0]) {
			i_info[I_LAST] = coul_drv_battery_fifo_curr(sync_num);
			i_info[I_LAST] = -i_info[I_LAST] / UA_PER_MA;
			if (i_info[I_LAST] != i_info[I_HEAD])
				sync_num++;
		}
		sync_num = clamp_val(sync_num, 0, i_info[I_FIF0]);
		for (i = sync_num - 1; i >= 0; i--) {
			/* avoid data lost when left time is not divisible by fifo interval */
			node.sample_time = l_info[L_SAMPLE] +
				di->fifo_interval * (sync_num - i);
			i_info[I_CUR] = coul_drv_battery_fifo_curr(i);
			node.current_ma = -(i_info[I_CUR] / UA_PER_MA);
			node.duration = di->fifo_interval;
			node.temperature =  coul_drv_battery_temperature();
			node.soc_now = coul_drv_battery_ufcapacity_tenth();
			node.list.next = NULL;
			node.list.prev = NULL;
			polar_debug("%s:time:%lu,curr:%d,duration:%lu,temp:%d:soc:%d\n",
				__func__, node.sample_time, node.current_ma,
				node.duration, node.temperature, node.soc_now);
			/* here we put the fifo info to 35S ringbuffer */
			fill_up_polar_fifo(di, &node, &di->coul_fifo_head.list,
				di->fifo_buffer, COUL_FIFO_SAMPLE_TIME);
		}
		/* modify the sample timer when we have synchronized the polar info */
		l_info[L_MS] = di->fifo_interval * i_info[I_FIF0];
		kt = ktime_set(l_info[L_MS] / MSEC_PER_SEC,
			(l_info[L_MS] % MSEC_PER_SEC) * NSEC_PER_MSEC);
		if (!hrtimer_active(&di->coul_sample_timer))
			hrtimer_forward_now(&di->coul_sample_timer, kt);
	}
	/* copy 35s fifo buffer to 20min polar buffer */
	copy_fifo_buffer(&di->coul_fifo_head.list, &di->polar_head.list,
		di->fifo_buffer, di->polar_buffer);
	spin_unlock_irqrestore(&di->coul_fifo_lock, l_info[L_FLAG]);
}
EXPORT_SYMBOL(sync_sample_info);

bool is_polar_list_ready(void)
{
	struct hisi_polar_device *di = g_polar_di;
	struct list_head *head = NULL;
	struct ploarized_info *ppolar_head = NULL;

	if (!di)
		return FALSE;
	head = &di->polar_head.list;
	if (!list_empty(head)) {
		ppolar_head = list_first_entry(head,
			struct ploarized_info, list);
		if (time_after(ppolar_head->sample_time,
			COUL_POLAR_SAMPLE_TIME))
			return TRUE;
	}
	return FALSE;
}
EXPORT_SYMBOL(is_polar_list_ready);

void get_resume_polar_info(struct polar_com_para *para,
	int eco_ibat, int duration, int sample_time)
{
	struct hisi_polar_device *di = g_polar_di;
	struct ploarized_info node, node_eco;

	if (!di || duration <= 0)
		return;
	/* sample for Tn-1 */
	if (eco_ibat)
		node.sample_time = sample_time - di->fifo_interval;
	else
		node.sample_time = sample_time;
	node.current_ma = para->cur;
	node.duration = duration;
	node.temperature = para->temp;
	node.soc_now = para->soc;
	node.list.next = NULL;
	node.list.prev = NULL;
	polar_debug("%s:time:%lu,curr:%d,duration:%lu,temp:%d:soc:%d\n",
		__func__, node.sample_time, node.current_ma, node.duration,
		node.temperature, node.soc_now);
	fill_up_polar_fifo(di, &node, &di->coul_fifo_head.list,
		di->fifo_buffer, COUL_FIFO_SAMPLE_TIME);
	/* sample for Tn */
	if (eco_ibat) {
		node_eco.sample_time = sample_time;
		node_eco.current_ma = eco_ibat / UA_PER_MA;
		node_eco.duration = di->fifo_interval;
		node_eco.temperature = para->temp;
		node_eco.soc_now = para->soc;
		node_eco.list.next = NULL;
		node_eco.list.prev = NULL;
		polar_debug("%s:time:%lu,curr:%d,duration:%lu,temp:%d:soc:%d\n",
			__func__, node_eco.sample_time, node_eco.current_ma,
			node_eco.duration, node_eco.temperature, node_eco.soc_now);
		fill_up_polar_fifo(di, &node_eco, &di->coul_fifo_head.list,
			di->fifo_buffer, COUL_FIFO_SAMPLE_TIME);
	}
	copy_fifo_buffer(&di->coul_fifo_head.list, &di->polar_head.list,
		di->fifo_buffer, di->polar_buffer);
}
EXPORT_SYMBOL(get_resume_polar_info);

void start_polar_sample(void)
{
	struct hisi_polar_device *di = g_polar_di;
	ktime_t kt;
	unsigned long fifo_time_ms;
	int fifo_depth;

	if (!di)
		return;
	fifo_depth = coul_drv_battery_fifo_depth();
	fifo_time_ms = di->fifo_interval * fifo_depth;
	kt = ktime_set(fifo_time_ms / MSEC_PER_SEC,
		(fifo_time_ms % MSEC_PER_SEC) * NSEC_PER_MSEC);
	hrtimer_start(&di->coul_sample_timer, kt, HRTIMER_MODE_REL);
}
EXPORT_SYMBOL(start_polar_sample);

void stop_polar_sample(void)
{
	struct hisi_polar_device *di = g_polar_di;

	if (!di)
		return;
	hrtimer_cancel(&di->coul_sample_timer);
}
EXPORT_SYMBOL(stop_polar_sample);

/* update polar data and power supply capability */
int polar_params_calculate(struct polar_calc_info *polar, int ocv_soc_mv,
	int vol_now, int cur, bool update_a)
{
	int curr_future_5s, curr_future_peak, batt_soc_real, temp;
	long polar_future_5s;
	struct hisi_polar_device *di = g_polar_di;
	struct polar_com_para para = {0};

	if (!di || !polar)
		return -1;
	batt_soc_real =  coul_drv_battery_ufcapacity_tenth();
	temp = coul_drv_battery_temperature();
	polar_info_calc(di, 0);
	/* get current polar vol */
	polar->vol = get_polar_vol(di, batt_soc_real, temp, cur);
	polar->vol = polar->vol - g_polar_err_b2;
	/* pull 5s ahead to calculate future polar vol */
	polar_info_calc(di, POLAR_CURR_PREDICT_MSECS);
	polar_future_5s =
		calculate_polar_volatge(&g_polar_res_lut5, batt_soc_real, temp);

	para.ocv_soc_mv = ocv_soc_mv;
	para.temp = temp;
	para.soc = batt_soc_real;
	para.cur = cur;
	if (update_a)
		update_polar_error_a(&para, vol_now, polar->vol);

	/* calculate future max avg current */
	curr_future_5s = get_estimate_max_avg_curr(&para,
		polar_future_5s, di->v_cutoff, di->r_pcb);
	/* calculate future max peak current */
	curr_future_peak = get_estimate_peak_curr(ocv_soc_mv,
		batt_soc_real, temp, di->v_cutoff, di->r_pcb);
	if (curr_future_peak < curr_future_5s)
		curr_future_peak = curr_future_5s;
	polar->curr_5s = curr_future_5s;
	polar->curr_peak = curr_future_peak;
	polar->ocv_old = ocv_soc_mv;
	polar->ori_vol = vol_now;
	polar->ori_cur = cur;
	polar->last_calc_temp = temp;
	if (update_a) {
		polar->err_a = g_polar_err_a;
		update_polar_error_b(ocv_soc_mv, vol_now, polar->vol);
		record_polar_vol(polar->vol);
	}
	return 0;
}
EXPORT_SYMBOL(polar_params_calculate);

/* update polar ocv data */
int polar_ocv_params_calc(struct polar_calc_info *polar,
	int batt_soc_real, int temp, int cur)
{
	struct hisi_polar_device *di = g_polar_di;

	if (!di || !polar)
		return -1;

	polar_info_calc(di, 0);
	/* get current polar vol */
	polar->sr_polar_vol0 = get_polar_vol(di, batt_soc_real, temp, cur);
	polar_info_calc(di, -POLAR_CURR_PREDICT_MSECS);
	polar->sr_polar_vol1 =
		get_polar_vol(di, batt_soc_real, temp, di->last_avgcurr_5s);
	polar->sr_polar_err_a =
		get_trained_a(&g_polar_learn_lut, temp, batt_soc_real);
	polar_info("[%s]sr_polar_vol0:%d,sr_polar_vol1:%d,sr_polar_err_a:%d\n",
		__func__, polar->sr_polar_vol0, polar->sr_polar_vol1,
		polar->sr_polar_err_a);
	return 0;
}
EXPORT_SYMBOL(polar_ocv_params_calc);

/*
 * Clear the correction coefficient b value.
 * This parameter takes effect only when the OCV is updated.
 * Ensure that the process is mutually exclusive.
 */
void clear_polar_err_b(void)
{
	struct hisi_polar_device *di = g_polar_di;

	if (!di)
		return;
	g_polar_err_b1 = 0;
	g_polar_err_b2 = 0;
}
EXPORT_SYMBOL(clear_polar_err_b);

#ifdef CONFIG_HISI_DEBUG_FS
int test_vector_curr_lookup(int curr)
{
	return interpolate_curr_vector(g_polar_curr_vector_interval,
		POLAR_CURR_ARRAY_VECTOR_NUM, curr);
}

int test_nearest_lookup(int soc)
{
	return interpolate_nearest_x(g_polar_resistence_pc_points,
		POLAR_RES_PC_CURR_ROWS, soc);
}
#endif
static struct device_node *get_batt_phandle(struct device_node *np,
	const char *prop, int p_num, const char *name)
{
	int i, ret;
	const char *cell_brand = NULL;
	struct device_node *temp_node = NULL;

	if (!np)
		return NULL;

	if (p_num < 0) {
		polar_info("[%s]get phandle list count failed", __func__);
		return NULL;
	}
	/* iter the phandle list */
	for (i = 0; i < p_num; i++) {
		temp_node = of_parse_phandle(np, prop, i);
		ret = of_property_read_string(
			temp_node, "batt_brand", &cell_brand);
		if (ret) {
			polar_err("get batt_brand failed\n");
			return NULL;
		}
		if (strstr(name, cell_brand))
			break;
	}
	if (i == p_num) {
		polar_err("no battery modle matched\n");
		return NULL;
	}
	/* print the battery brand */
	polar_debug("batt_name:%s, cell_name:%s matched\n", name, cell_brand);
	return temp_node;
}

static int get_polar_table_info(struct device_node *bat_node,
	const char *propname, u32 *outvalues, int tbl_size)
{
	int ele_count;
	int ret;
	/* get polar_ocv_table from dts */
	ele_count = of_property_count_u32_elems(bat_node, propname);
	polar_info("%s:ele_cnt:%d\n", propname, ele_count);
	/* check if ele_count match with polar_ocv_table */
	if (ele_count != tbl_size) {
		polar_err("ele_count:%d mismatch with %s\n", ele_count, propname);
		return -EINVAL;
	}
	ret = of_property_read_u32_array(bat_node, propname,
		outvalues, ele_count);
	if (ret) {
		polar_err("get polar_ocv_table failed\n");
		return ret;
	}
	return 0;
}

static int polar_table_info_get(struct device_node *bat_node)
{
	int ret;

	ret = get_polar_table_info(bat_node,
		"polar_res_table", (u32 *)g_polar_res_lut.value,
		(POLAR_OCV_PC_TEMP_COLS * POLAR_RES_PC_CURR_ROWS
			* POLAR_CURR_ARRAY_NUM));
	if (ret)
		goto clr;
	ret = get_polar_table_info(bat_node,
		"polar_res0_table", (u32 *)g_polar_res_lut0.value,
		(POLAR_OCV_PC_TEMP_COLS * POLAR_RES_PC_CURR_ROWS
			* POLAR_CURR_ARRAY_NUM));
	if (ret)
		goto clr;
	ret = get_polar_table_info(bat_node,
		"polar_res1_table", (u32 *)g_polar_res_lut1.value,
		(POLAR_OCV_PC_TEMP_COLS * POLAR_RES_PC_CURR_ROWS
			* POLAR_CURR_ARRAY_NUM));
	if (ret)
		goto clr;

	ret = get_polar_table_info(bat_node,
		"polar_res5_table", (u32 *)g_polar_res_lut5.value,
		(POLAR_OCV_PC_TEMP_COLS * POLAR_RES_PC_CURR_ROWS
			* POLAR_CURR_ARRAY_NUM));
	if (ret)
		goto clr;

	ret = get_polar_table_info(bat_node,
		"polar_vector_table", (u32 *)g_polar_vector_lut.value,
		(POLAR_OCV_PC_TEMP_COLS * POLAR_RES_PC_CURR_ROWS
			* POLAR_CURR_ARRAY_VECTOR_NUM * POLAR_VECTOR_SIZE));
	if (ret)
		goto clr;
	polar_info("%s:get polar dts info success\n", __func__);
	return 0;
clr:
	memset_s(g_polar_ocv_lut.ocv, sizeof(g_polar_ocv_lut.ocv),
		 0, sizeof(g_polar_ocv_lut.ocv));
	memset_s(g_polar_res_lut.value, sizeof(g_polar_res_lut.value),
		 0, sizeof(g_polar_res_lut.value));
	memset_s(g_polar_res_lut0.value, sizeof(g_polar_res_lut0.value),
		 0, sizeof(g_polar_res_lut0.value));
	memset_s(g_polar_res_lut1.value, sizeof(g_polar_res_lut1.value),
		 0, sizeof(g_polar_res_lut1.value));
	memset_s(g_polar_res_lut5.value, sizeof(g_polar_res_lut5.value),
		 0, sizeof(g_polar_res_lut5.value));
	memset_s(g_polar_vector_lut.value, sizeof(g_polar_vector_lut.value),
		 0, sizeof(g_polar_vector_lut.value));
	return ret;
}

static void get_polar_ishort_dts(struct hisi_polar_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "ishort_enable",
		(unsigned int *)&di->ishort_info.enable);
	if (ret) {
		di->ishort_info.enable = 0;
		polar_err("get polar_ishort_enable fail\n");
	}

	if (di->ishort_info.enable == 0)
		return;
	ret = of_property_read_u32(np, "ishort_time_interval",
		(unsigned int *)&di->ishort_info.time_interval);
	if (ret) {
		di->ishort_info.time_interval =
			coul_minutes(DEFAULT_ISHORT_TIME_INTERVAL);
		polar_err("get ishort_time_interval fail\n");
	}
	ret = of_property_read_u32(np, "ishort_dischg_interval",
		(unsigned int *)&di->ishort_info.dischg_interval);
	if (ret) {
		/* use 10 percent soc as default value */
		di->ishort_info.dischg_interval = DEFAULT_DISCHG_INTERVAL;
		polar_err("get ishort_dischg_interval fail\n");
	}
	ret = of_property_read_u32(np, "ishort_report_curr",
		(unsigned int *)&di->ishort_info.report_curr_ma);
	if (ret) {
		/* use 50 ma as default report limit */
		di->ishort_info.report_curr_ma = DEFAULT_ISHORT_CURRENT_MA;
		polar_err("get ishort_report_curr fail\n");
	}
}

static int get_polar_dts_info(struct hisi_polar_device *di, const char *name)
{
	int ret = 0;
	int ret1 = 0;
	int batt_count = 0;
	struct device_node *bat_node = NULL;
	struct device_node *coul_node = NULL;

	if (!di || !name)
		return -EINVAL;
	coul_node = of_find_compatible_node(NULL, NULL, "hisilicon,coul_core");
	if (coul_node) {
		get_polar_ishort_dts(di, coul_node);
		ret = of_property_read_u32(coul_node, "normal_cutoff_vol_mv",
			&di->v_cutoff);
		ret1 = of_property_read_u32(coul_node, "r_pcb", &di->r_pcb);
	}
	if (!coul_node || ret || ret1) {
		di->v_cutoff = BATTERY_NORMAL_CUTOFF_VOL;
		di->r_pcb = DEFAULT_RPCB;
		polar_err("get coul info failed\n");
		return -1;
	}

	ret = of_property_read_u32(di->np, "fifo_interval", &di->fifo_interval);
	if (ret) {
		di->fifo_interval = COUL_DEFAULT_SAMPLE_INTERVAL;
		polar_err("get fifo_interval failed\n");
		return ret;
	}
	polar_info("fifo_interval:%u\n", di->fifo_interval);
	ret = of_property_read_u32(di->np, "fifo_depth", &di->fifo_depth);
	if (ret) {
		polar_err("get fifo_depth failed\n");
		return ret;
	}
	polar_info("fifo_depth:%u\n", di->fifo_depth);
	ret = of_property_read_u32(di->np, "polar_batt_cnt", &batt_count);
	if (ret) {
		polar_err("get fifo_depth failed\n");
		return ret;
	}
	polar_info("polar_batt_cnt:%d\n", batt_count);
	bat_node = get_batt_phandle(di->np, "polar_batt_name", batt_count, name);
	if (!bat_node) {
		polar_err("get polar_phandle failed\n");
		ret = -EINVAL;
		goto out;
	}
	ret = polar_table_info_get(bat_node);
out:
	return ret;
}

#ifdef CONFIG_HISI_DEBUG_FS
static DEVICE_ATTR(self_learn_value, (S_IRUSR | S_IRGRP),
	polar_self_learn_show, NULL);
#endif

static void polar_lut_clear(void)
{
	memset_s(g_polar_ocv_lut.ocv, sizeof(g_polar_ocv_lut.ocv),
		 0, sizeof(g_polar_ocv_lut.ocv));
	memset_s(g_polar_res_lut.value, sizeof(g_polar_res_lut.value),
		 0, sizeof(g_polar_res_lut.value));
	memset_s(g_polar_res_lut0.value, sizeof(g_polar_res_lut0.value),
		 0, sizeof(g_polar_res_lut0.value));
	memset_s(g_polar_res_lut1.value, sizeof(g_polar_res_lut1.value),
		 0, sizeof(g_polar_res_lut1.value));
	memset_s(g_polar_res_lut5.value, sizeof(g_polar_res_lut5.value),
		 0, sizeof(g_polar_res_lut5.value));
	memset_s(g_polar_vector_lut.value, sizeof(g_polar_vector_lut.value),
		 0, sizeof(g_polar_vector_lut.value));
	memset_s(g_polar_avg_curr_info, sizeof(g_polar_avg_curr_info),
		 0, sizeof(g_polar_avg_curr_info));
}

static int polar_info_init(struct hisi_polar_device *di)
{
	int i, ret;
	char batt_name[BATTCELL_NAME_SIZE_MAX] = {0};
	int batt_fcc = coul_drv_battery_fcc();
	int batt_fcc_design = coul_drv_battery_fcc_design();
	int batt_present = coul_drv_is_battery_exist();
	char *batt_brand = coul_drv_battery_brand();

	if (!batt_present)
		return -1;
	for (i = 1; i < POLAR_CURR_ARRAY_NUM; i++)
		g_polar_curr_interval[i] =
			g_polar_curr_interval[i] * batt_fcc / UA_PER_MA;

	for (i = 1; i < POLAR_CURR_ARRAY_VECTOR_NUM; i++)
		g_polar_curr_vector_interval[i] =
			g_polar_curr_vector_interval[i] * batt_fcc / UA_PER_MA;

	polar_learn_lut_init(&g_polar_learn_lut);
	di->polar_vol_index = 0;
	for (i = 0; i < POLAR_ARRAY_NUM; i++)
		di->polar_vol_array[i] = POLAR_VOL_INVALID;
	polar_lut_clear();
#ifdef CONFIG_HISI_DEBUG_FS
	ret = device_create_file(di->dev, &dev_attr_self_learn_value);
	if (ret)
		polar_err("failed to create file");
#endif
	polar_debug("cell name:%s_%d\n", batt_brand, batt_fcc_design);
	ret = snprintf_s(batt_name, BATTCELL_NAME_SIZE_MAX,
		 BATTCELL_NAME_SIZE_MAX - 1, "%s_%d", batt_brand, batt_fcc_design);
	if (ret < 0) {
		polar_err("get batt cell name fail\n");
		return ret;
	}
	ret = get_polar_dts_info(di, batt_name);
	if (ret) {
		polar_err("get dts info failed\n");
		return ret;
	}
	if ((di->polar_buffer->max_num * di->fifo_interval) <= COUL_POLAR_SAMPLE_TIME) {
		polar_err("buffer is not enough for sample:max_node:%u,fifo_time:%u",
			  di->polar_buffer->max_num, di->fifo_interval);
		return -1;
	}
	if ((di->fifo_buffer->max_num * di->fifo_interval) <= COUL_FIFO_SAMPLE_TIME) {
		polar_err("buffer is not enough for sample:max_node:%u,fifo_time:%u",
			  di->fifo_buffer->max_num, di->fifo_interval);
		return -1;
	}
	return 0;
}

static int polar_buffer_alloc(struct hisi_polar_device *di)
{
	int retval;

	di->polar_buffer =
		(struct hisiap_ringbuffer_s *)devm_kzalloc(di->dev,
			POLAR_BUFFER_SIZE, GFP_KERNEL);
	di->fifo_buffer =
		(struct hisiap_ringbuffer_s *)devm_kzalloc(di->dev,
			FIFO_BUFFER_SIZE, GFP_KERNEL);
	if (!di->polar_buffer || !di->fifo_buffer) {
		polar_err("failed to alloc polar_buffer struct\n");
		return -ENOMEM;
	}

	polar_info("polar_buffer alloc ok:%pK", di->polar_buffer);
	retval = hisiap_ringbuffer_init(di->polar_buffer,
		POLAR_BUFFER_SIZE, sizeof(struct ploarized_info), "coul_polar");
	retval |= hisiap_ringbuffer_init(di->fifo_buffer,
		FIFO_BUFFER_SIZE, sizeof(struct ploarized_info), "coul_fifo");
	return retval;
}

static int hisi_coul_polar_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	int fifo_depth, retval0, retval1;
	struct hisi_polar_device *di = NULL;
	ktime_t kt;
	unsigned long fifo_time_ms;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	di->dev = &pdev->dev;
	di->np = node;
	retval0 = polar_buffer_alloc(di);
	retval1 = polar_info_init(di);
	if (retval0 || retval1) {
		platform_set_drvdata(pdev, NULL);
		g_polar_di = NULL;
		polar_err("%s failed to init polar info!!!\n", __func__);
		return -ENOMEM;
	}
	di->polar_dev_ops = get_polar_dev_ops();
	if (polar_dev_ops_check_fail(di))
		return -ENOMEM;
	INIT_LIST_HEAD(&di->polar_head.list);
	INIT_LIST_HEAD(&di->coul_fifo_head.list);
	spin_lock_init(&di->coul_fifo_lock);
	mutex_init(&di->polar_vol_lock);
	fifo_depth = coul_drv_battery_fifo_depth();
	fifo_time_ms = di->fifo_interval * fifo_depth;
	kt = ktime_set(fifo_time_ms / MSEC_PER_SEC,
		(fifo_time_ms % MSEC_PER_SEC) * NSEC_PER_MSEC);
	hrtimer_init(&di->coul_sample_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->coul_sample_timer.function = sample_timer_func;
	hrtimer_start(&di->coul_sample_timer, kt, HRTIMER_MODE_REL);
	polar_ipc_init(di);
	platform_set_drvdata(pdev, di);
	g_polar_di = di;
	polar_info("Hisi coul polar ready\n");
	return 0;
}

static int hisi_coul_polar_remove(struct platform_device *pdev)
{
	struct hisi_polar_device *di = platform_get_drvdata(pdev);

	if (di == NULL) {
		polar_err("polar dev is null\n");
	} else {
		hrtimer_cancel(&di->coul_sample_timer);
		mutex_destroy(&di->polar_vol_lock);
	}

	g_polar_di = NULL;
#ifdef CONFIG_HISI_DEBUG_FS
	device_remove_file(&pdev->dev, &dev_attr_self_learn_value);
#endif
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id hisi_coul_polar_of_match[] = {
	{
		.compatible = "hisi,coul_polar",
		.data = NULL
	},
	{
	},
};

MODULE_DEVICE_TABLE(of, hisi_coul_polar_of_match);

static struct platform_driver hisi_coul_polar_driver = {
	.probe = hisi_coul_polar_probe,
	.driver = {
		.name = "coul_polar",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_coul_polar_of_match),
	},
	.remove = hisi_coul_polar_remove,
};

static int __init hisi_coul_polar_init(void)
{
	platform_driver_register(&hisi_coul_polar_driver);
	return 0;
}

fs_initcall(hisi_coul_polar_init);

static void __exit hisi_coul_polar_exit(void)
{
	platform_driver_unregister(&hisi_coul_polar_driver);
}

module_exit(hisi_coul_polar_exit);

MODULE_DESCRIPTION("COUL POLARIZATION driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

