/*
 * ILITEK Touch IC driver
 *
 * Copyright (C) 2011 ILI Technology Corporation.
 *
 * Author: Dicky Chiang <dicky_chiang@ilitek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ilitek.h"

#define USER_STR_BUFF PAGE_SIZE
#define IOCTL_I2C_BUFF PAGE_SIZE
#define ILITEK_IOCTL_MAGIC 100
#define ILITEK_IOCTL_MAXNR 27

#define ILITEK_IOCTL_I2C_WRITE_DATA _IOWR(ILITEK_IOCTL_MAGIC, 0, u8*)
#define ILITEK_IOCTL_I2C_SET_WRITE_LENGTH _IOWR(ILITEK_IOCTL_MAGIC, 1, int)
#define ILITEK_IOCTL_I2C_READ_DATA _IOWR(ILITEK_IOCTL_MAGIC, 2, u8*)
#define ILITEK_IOCTL_I2C_SET_READ_LENGTH _IOWR(ILITEK_IOCTL_MAGIC, 3, int)

#define ILITEK_IOCTL_TP_HW_RESET _IOWR(ILITEK_IOCTL_MAGIC, 4, int)
#define ILITEK_IOCTL_TP_POWER_SWITCH _IOWR(ILITEK_IOCTL_MAGIC, 5, int)
#define ILITEK_IOCTL_TP_REPORT_SWITCH _IOWR(ILITEK_IOCTL_MAGIC, 6, int)
#define ILITEK_IOCTL_TP_IRQ_SWITCH _IOWR(ILITEK_IOCTL_MAGIC, 7, int)

#define ILITEK_IOCTL_TP_DEBUG_LEVEL _IOWR(ILITEK_IOCTL_MAGIC, 8, int)
#define ILITEK_IOCTL_TP_FUNC_MODE _IOWR(ILITEK_IOCTL_MAGIC, 9, int)

#define ILITEK_IOCTL_TP_FW_VER _IOWR(ILITEK_IOCTL_MAGIC, 10, u8*)
#define ILITEK_IOCTL_TP_PL_VER _IOWR(ILITEK_IOCTL_MAGIC, 11, u8*)
#define ILITEK_IOCTL_TP_CORE_VER _IOWR(ILITEK_IOCTL_MAGIC, 12, u8*)
#define ILITEK_IOCTL_TP_DRV_VER _IOWR(ILITEK_IOCTL_MAGIC, 13, u8*)
#define ILITEK_IOCTL_TP_CHIP_ID _IOWR(ILITEK_IOCTL_MAGIC, 14, u32*)

#define ILITEK_IOCTL_TP_NETLINK_CTRL _IOWR(ILITEK_IOCTL_MAGIC, 15, int*)
#define ILITEK_IOCTL_TP_NETLINK_STATUS _IOWR(ILITEK_IOCTL_MAGIC, 16, int*)

#define ILITEK_IOCTL_TP_MODE_CTRL _IOWR(ILITEK_IOCTL_MAGIC, 17, u8*)
#define ILITEK_IOCTL_TP_MODE_STATUS _IOWR(ILITEK_IOCTL_MAGIC, 18, int*)
#define ILITEK_IOCTL_ICE_MODE_SWITCH _IOWR(ILITEK_IOCTL_MAGIC, 19, int)

#define ILITEK_IOCTL_TP_INTERFACE_TYPE _IOWR(ILITEK_IOCTL_MAGIC, 20, u8*)
#define ILITEK_IOCTL_TP_DUMP_FLASH _IOWR(ILITEK_IOCTL_MAGIC, 21, int)
#define ILITEK_IOCTL_TP_FW_UART_CTRL _IOWR(ILITEK_IOCTL_MAGIC, 22, u8*)
#define ILITEK_IOCTL_TP_PANEL_INFO _IOWR(ILITEK_IOCTL_MAGIC, 23, u32*)
#define ILITEK_IOCTL_TP_INFO _IOWR(ILITEK_IOCTL_MAGIC, 24, u32*)
#define ILITEK_IOCTL_WRAPPER_RW  _IOWR(ILITEK_IOCTL_MAGIC, 25, u8*)
#define ILITEK_IOCTL_DDI_WRITE _IOWR(ILITEK_IOCTL_MAGIC, 26, u8*)
#define ILITEK_IOCTL_DDI_READ _IOWR(ILITEK_IOCTL_MAGIC, 27, u8*)

#ifdef CONFIG_COMPAT
#define ILITEK_COMPAT_IOCTL_I2C_WRITE_DATA _IOWR(ILITEK_IOCTL_MAGIC, 0, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_I2C_SET_WRITE_LENGTH _IOWR(ILITEK_IOCTL_MAGIC, 1, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_I2C_READ_DATA _IOWR(ILITEK_IOCTL_MAGIC, 2, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_I2C_SET_READ_LENGTH _IOWR(ILITEK_IOCTL_MAGIC, 3, compat_uptr_t)

#define ILITEK_COMPAT_IOCTL_TP_HW_RESET _IOWR(ILITEK_IOCTL_MAGIC, 4, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_POWER_SWITCH _IOWR(ILITEK_IOCTL_MAGIC, 5, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_REPORT_SWITCH _IOWR(ILITEK_IOCTL_MAGIC, 6, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_IRQ_SWITCH _IOWR(ILITEK_IOCTL_MAGIC, 7, compat_uptr_t)

#define ILITEK_COMPAT_IOCTL_TP_DEBUG_LEVEL _IOWR(ILITEK_IOCTL_MAGIC, 8, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_FUNC_MODE _IOWR(ILITEK_IOCTL_MAGIC, 9, compat_uptr_t)

#define ILITEK_COMPAT_IOCTL_TP_FW_VER _IOWR(ILITEK_IOCTL_MAGIC, 10, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_PL_VER _IOWR(ILITEK_IOCTL_MAGIC, 11, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_CORE_VER _IOWR(ILITEK_IOCTL_MAGIC, 12, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_DRV_VER _IOWR(ILITEK_IOCTL_MAGIC, 13, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_CHIP_ID _IOWR(ILITEK_IOCTL_MAGIC, 14, compat_uptr_t)

#define ILITEK_COMPAT_IOCTL_TP_NETLINK_CTRL _IOWR(ILITEK_IOCTL_MAGIC, 15, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_NETLINK_STATUS _IOWR(ILITEK_IOCTL_MAGIC, 16, compat_uptr_t)

#define ILITEK_COMPAT_IOCTL_TP_MODE_CTRL _IOWR(ILITEK_IOCTL_MAGIC, 17, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_MODE_STATUS _IOWR(ILITEK_IOCTL_MAGIC, 18, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_ICE_MODE_SWITCH _IOWR(ILITEK_IOCTL_MAGIC, 19, compat_uptr_t)

#define ILITEK_COMPAT_IOCTL_TP_INTERFACE_TYPE _IOWR(ILITEK_IOCTL_MAGIC, 20, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_DUMP_FLASH _IOWR(ILITEK_IOCTL_MAGIC, 21, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_FW_UART_CTRL _IOWR(ILITEK_IOCTL_MAGIC, 22, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_PANEL_INFO _IOWR(ILITEK_IOCTL_MAGIC, 23, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_TP_INFO _IOWR(ILITEK_IOCTL_MAGIC, 24, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_WRAPPER_RW _IOWR(ILITEK_IOCTL_MAGIC, 25, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_DDI_WRITE _IOWR(ILITEK_IOCTL_MAGIC, 26, compat_uptr_t)
#define ILITEK_COMPAT_IOCTL_DDI_READ _IOWR(ILITEK_IOCTL_MAGIC, 27, compat_uptr_t)
#endif

struct record_state {
	u8 touch_palm_state_e : 2;
	u8 app_an_statu_e : 3;
	u8 app_sys_check_bg_abnormal : 1;
	u8 g_b_wrong_bg : 1;
};

static unsigned char g_user_buf[USER_STR_BUFF] = {0};

int ili_str2hex(char *str)
{
	int strlen, result, intermed, intermedtop;
	char *s = str;

	while (*s != 0x0)
		s++;

	strlen = (int)(s - str);
	s = str;
	if (*s != 0x30) {
		return -1;
	}

	s++;

	if (*s != 0x78 && *s != 0x58) {
		return -1;
	}
	s++;

	strlen = strlen - 3;
	result = 0;
	while (*s != 0x0) {
		intermed = *s & 0x0f;
		intermedtop = *s & 0xf0;
		if (intermedtop == 0x60 || intermedtop == 0x40)
			intermed += 0x09;
		intermed = intermed << (strlen << 2);
		result = result | intermed;
		strlen -= 1;
		s++;
	}
	return result;
}

int ili_katoi(char *str, int length)
{
	int result = 0;
	unsigned int digit;
	int sign;

	if (*str == '-') {
		sign = 1;
		str += 1;
	} else {
		sign = 0;
		if (*str == '+')
			str += 1;
	}

	for (;; str += 1) {
		digit = *str - '0';
		if (digit > 9)
			break;
		result = (10 * result) + digit;
	}

	if (sign) {
		return -result;
	}
	return result;
}

struct file_buffer {
	char *ptr;
	char fname[128];
	int32_t wlen;
	int32_t flen;
	int32_t max_size;
};

static int file_write(struct file_buffer *file, bool new_open)
{
	struct file *f = NULL;
	mm_segment_t fs;
	loff_t pos;

	if (file->ptr == NULL) {
		TS_LOG_ERR("str is invaild\n");
		return -1;
	}

	if (file->fname == NULL) {
		TS_LOG_ERR("file name is invaild\n");
		return -1;
	}

	if (file->flen >= file->max_size) {
		TS_LOG_ERR("The length saved to file is too long !\n");
		return -1;
	}

	if (new_open)
		f = filp_open(file->fname, O_WRONLY | O_CREAT | O_TRUNC, 644);
	else
		f = filp_open(file->fname, O_WRONLY | O_CREAT | O_APPEND, 644);

	if (err_alloc_mem(f)) {
		TS_LOG_ERR("Failed to open %s file\n", file->fname);
		return -1;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_write(f, file->ptr, file->flen, &pos);
	set_fs(fs);
	filp_close(f, NULL);
	return 0;
}

static int ilitek_debug_node_buff_control(bool open)
{
	int i, ret;

	ilits->dnp = open;

	TS_LOG_INFO("Debug buf ctrl = %s\n", ilits->dnp ? "Enabled" : "Disabled");

	if (open) {
		ilits->dbf = 0;
		ilits->odi = 0;
		ipio_kfree((void **)&ilits->dbl);
		ilits->dbl = kzalloc(TR_BUF_LIST_SIZE * sizeof(*ilits->dbl), GFP_KERNEL);
		if (err_alloc_mem(ilits->dbl)) {
			TS_LOG_ERR("Failed to allocate ilits->dbl mem, %ld\n", PTR_ERR(ilits->dbl));
			ret = -ENOMEM;
			goto out;
		}

		for (i = 0; i < TR_BUF_LIST_SIZE; i++) {
			ilits->dbl[i].mark = false;
			ipio_kfree((void **)&ilits->dbl[i].data);
			ilits->dbl[i].data = kzalloc(TR_BUF_SIZE * sizeof(unsigned char), GFP_KERNEL);
			if (err_alloc_mem(ilits->dbl[i].data)) {
				TS_LOG_ERR("Failed to allocate dbl[%d] mem, %ld\n", i, PTR_ERR(ilits->dbl[i].data));
				ret = -ENOMEM;
				goto out;
			}
		}
		return 0;
	}

out:
	for (i = 0; i < TR_BUF_LIST_SIZE; i++) {
		ilits->dbl[i].mark = false;
		ipio_kfree((void **)&ilits->dbl[i].data);
	}
	ipio_kfree((void **)&ilits->dbl);
	return ret;
}

static int debug_mode_get_data(struct file_buffer *file, u8 type, u32 frame_count)
{
	int ret;
	u8 cmd[2] = { 0 }, row, col;
	s16 temp;
	unsigned char *ptr = NULL;
	int j;
	u16 write_index = 0;

	ilitek_debug_node_buff_control(DISABLE);

	ilits->dbf = 0;
	row = ilits->ych_num;
	col = ilits->xch_num;
	mutex_lock(&ilits->touch_mutex);

	cmd[0] = 0xFA;
	cmd[1] = type;
	ret = ilits->wrapper(cmd, 2, NULL, 0, ON, OFF);

	if (ilitek_debug_node_buff_control(ENABLE) < 0) {
		TS_LOG_ERR("Failed to allocate debug buf\n");
		ret = -ENOMEM;
		goto out;
	}

	mutex_unlock(&ilits->touch_mutex);

	if (ret < 0) {
		TS_LOG_ERR("Write 0xFA,0x%x failed\n", type);
		goto out;
	}

	while (write_index < frame_count) {
		file->wlen = 0;
		TS_LOG_INFO("frame = %d,index = %d,count = %d\n", write_index, ilits->odi, ilits->dbf);
		if (!wait_event_interruptible_timeout(ilits->inq, ilits->dbl[ilits->odi].mark, msecs_to_jiffies(3000))) {
			TS_LOG_ERR("debug mode get data timeout!\n");
			goto out;
		}

		mutex_lock(&ilits->touch_mutex);

		memset(file->ptr, 0, file->max_size);
		file->wlen += snprintf(file->ptr + file->wlen, (file->max_size - file->wlen), "\n\nFrame%d,", write_index);
		for (j = 0; j < col; j++)
			file->wlen += snprintf(file->ptr + file->wlen, (file->max_size - file->wlen), "[X%d] ,", j);
		ptr = &ilits->dbl[ilits->odi].data[35];
		for (j = 0; j < row * col; j++, ptr += 2) {
			temp = (*ptr << 8) + *(ptr + 1);
			if (j % col == 0)
				file->wlen += snprintf(file->ptr + file->wlen, (file->max_size - file->wlen), "\n[Y%d] ,", (j / col));
			file->wlen += snprintf(file->ptr + file->wlen, (file->max_size - file->wlen), "%d, ", temp);
		}
		file->wlen += snprintf(file->ptr + file->wlen, (file->max_size - file->wlen), "\n[X] ,");
		for (j = 0; j < row + col; j++, ptr += 2) {
			temp = (*ptr << 8) + *(ptr + 1);
			if (j == col)
				file->wlen += snprintf(file->ptr + file->wlen, (file->max_size - file->wlen), "\n[Y] ,");
			file->wlen += snprintf(file->ptr + file->wlen, (file->max_size - file->wlen), "%d, ", temp);
		}
		file_write(file, false);
		write_index++;

		mutex_unlock(&ilits->touch_mutex);

		ilits->dbl[ilits->odi].mark = false;
		ilits->odi = ((ilits->odi + 1) % TR_BUF_LIST_SIZE);
	}
out:
	ilitek_debug_node_buff_control(DISABLE);
	return ret;
}

int dev_mkdir(char *name, umode_t mode)
{
	int err;
	mm_segment_t fs;

	TS_LOG_INFO("mkdir: %s\n", name);
	fs = get_fs();
	set_fs(KERNEL_DS);
	err = sys_mkdir(name, mode);
	set_fs(fs);

	return err;
}

static ssize_t ilitek_proc_fw_pc_counter_read(struct file *pFile, char __user *buf, size_t size, loff_t *pos)
{
	int len = 0;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, USER_STR_BUFF * sizeof(unsigned char));

	ili_ic_get_pc_counter(0);
	len = snprintf(g_user_buf, PAGE_SIZE, "pc = 0x%x, latch = 0x%x\n", ilits->fw_pc, ilits->fw_latch);

	if (copy_to_user(buf, g_user_buf, len))
		TS_LOG_ERR("Failed to copy data to user space\n");

	*pos += len;
	return len;
}

static u32 rw_reg[5] = {0};
static ssize_t ilitek_proc_rw_tp_reg_read(struct file *pFile, char __user *buf, size_t size, loff_t *pos)
{
	int ret = 0, len = 0;
	bool mcu_on = 0, read = 0;
	u32 type, addr, read_data, write_data, write_len, stop_mcu;
	bool esd_en = ilits->wq_esd_ctrl, bat_en = ilits->wq_bat_ctrl;

	if (*pos != 0)
		return 0;

	stop_mcu = rw_reg[0];
	type = rw_reg[1];
	addr = rw_reg[2];
	write_data = rw_reg[3];
	write_len = rw_reg[4];

	TS_LOG_INFO("stop_mcu = %d\n", rw_reg[0]);

	if (esd_en)
		ili_wq_ctrl(WQ_ESD, DISABLE);
	if (bat_en)
		ili_wq_ctrl(WQ_BAT, DISABLE);

	mutex_lock(&ilits->touch_mutex);

	memset(g_user_buf, 0, USER_STR_BUFF * sizeof(unsigned char));

	if (stop_mcu == mcu_on)
		ret = ili_ice_mode_ctrl(ENABLE, ON);
	else
		ret = ili_ice_mode_ctrl(ENABLE, OFF);

	if (ret < 0) {
		TS_LOG_ERR("Failed to enter ICE mode, ret = %d\n", ret);
		len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "%s\n", "Failed to enter ICE mode");
	}

	if (type == read) {
		if (ili_ice_mode_read(addr, &read_data, sizeof(u32)) < 0) {
			TS_LOG_ERR("Read data error\n");
			len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "%s\n", "Read data error");
		}

		TS_LOG_INFO("[READ]:addr = 0x%06x, read = 0x%08x\n", addr, read_data);
		len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "READ:addr = 0x%06x, read = 0x%08x\n", addr, read_data);
	} else {
		if (ili_ice_mode_write(addr, write_data, write_len) < 0) {
			TS_LOG_ERR("Write data error\n");
			len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "%s\n", "Write data error");
		}

		TS_LOG_INFO("[WRITE]:addr = 0x%06x, write = 0x%08x, len = %d byte\n", addr, write_data, write_len);
		len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "WRITE:addr = 0x%06x, write = 0x%08x, len =%d byte\n", addr, write_data, write_len);
	}

	if (stop_mcu == mcu_on)
		ret = ili_ice_mode_ctrl(DISABLE, ON);
	else
		ret = ili_ice_mode_ctrl(DISABLE, OFF);

	if (ret < 0) {
		TS_LOG_ERR("Failed to disable ICE mode, ret = %d\n", ret);
		len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "%s\n", "Failed to disable ICE mode");
	}

	if (copy_to_user((char *)buf, g_user_buf, len))
		TS_LOG_ERR("Failed to copy data to user space\n");

	if (esd_en)
		ili_wq_ctrl(WQ_ESD, ENABLE);
	if (bat_en)
		ili_wq_ctrl(WQ_BAT, ENABLE);

	*pos += len;
	mutex_unlock(&ilits->touch_mutex);
	return len;
}

static ssize_t ilitek_proc_rw_tp_reg_write(struct file *filp, const char *buff, size_t size, loff_t *pos)
{
	char *token = NULL, *cur = NULL;
	char cmd[256] = { 0 };
	u32 count = 0;

	if ((size - 1) > sizeof(cmd)) {
		TS_LOG_ERR("ERROR! input length is larger than local buffer\n");
		return -1;
	}

	mutex_lock(&ilits->touch_mutex);

	if (buff != NULL) {
		if (copy_from_user(cmd, buff, size - 1)) {
			TS_LOG_INFO("Failed to copy data from user space\n");
			size = -1;
			goto out;
		}
	}
	token = cur = cmd;
	while ((token = strsep(&cur, ",")) != NULL) {
		rw_reg[count] = ili_str2hex(token);
		TS_LOG_INFO("rw_reg[%d] = 0x%x\n", count, rw_reg[count]);
		count++;
	}

out:
	mutex_unlock(&ilits->touch_mutex);
	return size;
}

static ssize_t ilitek_proc_debug_switch_read(struct file *pFile, char __user *buff, size_t size, loff_t *pos)
{
	bool open = false;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, USER_STR_BUFF * sizeof(unsigned char));

	mutex_lock(&ilits->debug_mutex);

	open = !ilits->dnp;

	ilitek_debug_node_buff_control(open);

	size = snprintf(g_user_buf, USER_STR_BUFF * sizeof(unsigned char), "dnp : %s\n", ilits->dnp ? "Enabled" : "Disabled");
	*pos = size;

	if (copy_to_user(buff, g_user_buf, size))
		TS_LOG_ERR("Failed to copy data to user space\n");

	mutex_unlock(&ilits->debug_mutex);
	return size;
}

static ssize_t ilitek_proc_debug_message_read(struct file *filp, char __user *buff, size_t size, loff_t *pos)
{
	unsigned long p = *pos;
	int i = 0;
	int send_data_len = 0;
	int ret = 0;
	int data_count = 0;
	int one_data_bytes = 0;
	int need_read_data_len = 0;
	int type = 0;
	unsigned char *tmpbuf = NULL;
	unsigned char tmpbufback[128] = {0};

	if (filp->f_flags & O_NONBLOCK) {
		return -EAGAIN;
	}

	if (!ilits->dnp) {
		TS_LOG_ERR("Debug flag isn't enabled %d\n", ilits->dnp);
		return -EAGAIN;
	}

	mutex_lock(&ilits->debug_read_mutex);
	TS_LOG_DEBUG("f_count= %d, index = %d, mark = %d\n", ilits->dbf, ilits->odi, ilits->dbl[ilits->odi].mark);
	if (!wait_event_interruptible_timeout(ilits->inq, ilits->dbl[ilits->odi].mark, msecs_to_jiffies(3000))) {
		TS_LOG_ERR("WARNING ! there's no data received.\n");
		mutex_unlock(&ilits->debug_read_mutex);
		*pos = send_data_len;
		return send_data_len;
	}
	mutex_lock(&ilits->debug_mutex);

	tmpbuf = vmalloc(4096); /* buf size if even */
	if (err_alloc_mem(tmpbuf)) {
		TS_LOG_ERR("buffer vmalloc error\n");
		send_data_len += snprintf(tmpbufback + send_data_len, sizeof(tmpbufback), "buffer vmalloc error\n");
		ret = copy_to_user(buff, tmpbufback, send_data_len); /*ilits->dbl[0] */
		goto out;
	}

	if (ilits->dbl[ilits->odi].mark) {
		if (ilits->dbl[ilits->odi].data[0] == P5_X_DEMO_PACKET_ID) {
			need_read_data_len = 43;
		} else if (ilits->dbl[ilits->odi].data[0] == P5_X_I2CUART_PACKET_ID) {
			type = ilits->dbl[ilits->odi].data[3] & 0x0F;

			data_count = ilits->dbl[ilits->odi].data[1] * ilits->dbl[ilits->odi].data[2];

			if (type == 0 || type == 1 || type == 6) {
				one_data_bytes = 1;
			} else if (type == 2 || type == 3) {
				one_data_bytes = 2;
			} else if (type == 4 || type == 5) {
				one_data_bytes = 4;
			}
			need_read_data_len = data_count * one_data_bytes + 1 + 5;
		} else if (ilits->dbl[ilits->odi].data[0] == P5_X_DEBUG_PACKET_ID || ilits->dbl[ilits->odi].data[0] == P5_X_DEBUG_LITE_PACKET_ID) {
			send_data_len = 0;
			need_read_data_len = TR_BUF_SIZE - 8;
		}

		for (i = 0; i < need_read_data_len; i++) {
			send_data_len += snprintf(tmpbuf + send_data_len, sizeof(tmpbufback), "%02X", ilits->dbl[ilits->odi].data[i]);
			if (send_data_len >= 4096) {
				TS_LOG_ERR("send_data_len = %d set 4096 i = %d\n", send_data_len, i);
				send_data_len = 4096;
				break;
			}
		}

		send_data_len += snprintf(tmpbuf + send_data_len, sizeof(tmpbufback), "\n\n");

		if (p == 5 || size == 4096 || size == 2048) {
			ilits->dbl[ilits->odi].mark = false;
			ilits->odi = ((ilits->odi + 1) % TR_BUF_LIST_SIZE);
		}
	}

	/* Preparing to send debug data to user */
	if (size == 4096)
		ret = copy_to_user(buff, tmpbuf, send_data_len);
	else
		ret = copy_to_user(buff, tmpbuf + p, send_data_len - p);

	if (send_data_len <= 0 || send_data_len > 4096) {
		TS_LOG_ERR("send_data_len = %d set 4096\n", send_data_len);
		send_data_len = 4096;
	}

	if (ret) {
		TS_LOG_ERR("copy_to_user err\n");
		ret = -EFAULT;
	} else {
		*pos += send_data_len;
		ret = send_data_len;
		TS_LOG_DEBUG("Read %d bytes(s) from %ld\n", send_data_len, p);
	}

out:
	mutex_unlock(&ilits->debug_mutex);
	mutex_unlock(&ilits->debug_read_mutex);
	ipio_vfree((void **)&tmpbuf);
	return send_data_len;
}

static ssize_t ilitek_node_ver_info_read(struct file *filp, char __user *buff, size_t size, loff_t *pos)
{
	u32 len = 0;

	if (*pos != 0)
		return 0;

	mutex_lock(&ilits->touch_mutex);

	memset(g_user_buf, 0, USER_STR_BUFF * sizeof(unsigned char));

	len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "CHIP ID = %x\n", ilits->chip->id);
	len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "FW version = %d.%d.%d.%d\n",
			ilits->chip->fw_ver >> 24, (ilits->chip->fw_ver >> 16) & 0xFF,
			(ilits->chip->fw_ver >> 8) & 0xFF, ilits->chip->fw_ver & 0xFF);
	len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "FW MP version = %d.%d.%d.%d\n",
			ilits->chip->fw_mp_ver >> 24, (ilits->chip->fw_mp_ver >> 16) & 0xFF,
			(ilits->chip->fw_mp_ver >> 8) & 0xFF, ilits->chip->fw_mp_ver & 0xFF);

	if (ilits->protocol->core_ver_len == P5_X_CORE_VER_THREE_LENGTH)
		len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "Core version = %d.%d.%d\n",
			(ilits->chip->core_ver >> 24) & 0xFF, (ilits->chip->core_ver >> 16) & 0xFF,
			(ilits->chip->core_ver >> 8) & 0xFF);
	else
		len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "Core version = %d.%d.%d.%d\n",
			(ilits->chip->core_ver >> 24) & 0xFF, (ilits->chip->core_ver >> 16) & 0xFF,
			(ilits->chip->core_ver >> 8) & 0xFF, ilits->chip->core_ver & 0xFF);

	len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "Protocol version = %d.%d.%d\n",
			ilits->protocol->ver >> 16, (ilits->protocol->ver >> 8) & 0xFF,
			ilits->protocol->ver & 0xFF);
	len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "Driver version = %s\n", DRIVER_VERSION);

	if (copy_to_user((char *)buff, g_user_buf, len))
		TS_LOG_ERR("Failed to copy data to user space\n");

	*pos += len;
	mutex_unlock(&ilits->touch_mutex);
	return len;
}

static ssize_t ilitek_node_change_list_read(struct file *filp, char __user *buff, size_t size, loff_t *pos)
{
	u32 len = 0;

	if (*pos != 0)
		return 0;

	mutex_lock(&ilits->touch_mutex);

	memset(g_user_buf, 0, USER_STR_BUFF * sizeof(unsigned char));

	len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "============= Change list ==============\n");
	len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "[Drive version] = %s\n", DRIVER_VERSION);
	len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "[Patch] 20200702-0001,esd recovery issue\n");
	len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "[Patch] esd debug\n");
	len += snprintf(g_user_buf + len, USER_STR_BUFF - len, "========================================\n");

	if (copy_to_user((char *)buff, g_user_buf, len))
		TS_LOG_ERR("Failed to copy data to user space\n");

	*pos += len;
	mutex_unlock(&ilits->touch_mutex);
	return len;
}

int ili_get_tp_recore_ctrl(int data)
{
	int ret = 0;

	switch((int)data) {

	case ENABLE_RECORD:
		TS_LOG_INFO("recore enable");
		ret = ili_ic_func_ctrl("tp_recore", 1);
		mdelay(200);
		break;
	case DATA_RECORD:
		mdelay(50);
		TS_LOG_INFO("Get data");
		ret = ili_ic_func_ctrl("tp_recore", 2);
		if (ret < 0) {
			TS_LOG_ERR("cmd fail\n");
			goto out;
		}

		ret = ili_get_tp_recore_data();
		if (ret < 0)
			TS_LOG_ERR("get data fail\n");

		TS_LOG_INFO("recore reset");
		ret = ili_ic_func_ctrl("tp_recore", 3);
		if (ret < 0) {
			TS_LOG_ERR("cmd fail\n");
			goto out;
		}
		break;
	case DISABLE_RECORD:
		TS_LOG_INFO("recore disable");
		ret = ili_ic_func_ctrl("tp_recore", 0);
		break;

	}
out:
	return ret;

}

int ili_get_tp_recore_data(void)
{
	u8 buf[8] = {0}, record_case = 0;
	s8 index;
	u16 *raw = NULL, *raw_ptr = NULL, frame_len = 0;
	u32 base_addr = 0x20000, addr, len, *ptr, i, fcnt;
	struct record_state record_stat;
	bool ice = atomic_read(&ilits->ice_stat);

	if (ilits->wrapper(NULL, 0, buf, sizeof(buf), OFF, OFF) < 0) {
		TS_LOG_ERR("Get info fail\n");
		return -1;
	}

	addr = ((buf[0] << 8) | buf[1]) + base_addr;
	len = ((buf[2] << 8) | buf[3]);
	index = buf[4];
	fcnt = buf[5];
	record_case = buf[6];
	memcpy(&record_stat, &buf[7], 1);
	TS_LOG_INFO("addr = 0x%x, len = %d, lndex = 0x%d, fram num = %d, record_case = 0x%x\n",
		addr, len, index, fcnt, record_case);
	ili_dump_data(buf, 8, sizeof(buf), 0, "all record bytes");

	raw = kcalloc(len, sizeof(u8), GFP_ATOMIC);
	if (err_alloc_mem(raw)) {
		TS_LOG_ERR("Failed to allocate packet memory, %ld\n", PTR_ERR(raw));
		return -1;
	}
	ptr = (u32*)raw;

	if (!ice)
		ili_ice_mode_ctrl(ENABLE, ON);

	buf[0] = 0x25;
	buf[3] = (char)((addr & 0x00FF0000) >> 16);
	buf[2] = (char)((addr & 0x0000FF00) >> 8);
	buf[1] = (char)((addr & 0x000000FF));

	if (ilits->wrapper(buf, 4, NULL, 0, ON, OFF)) {
		TS_LOG_ERR("Failed to write iram data\n");
		return -ENODEV;
	}

	if (ilits->wrapper(NULL, 0, (u8 *)raw, len, ON, OFF)) {
		TS_LOG_ERR("Failed to Read iram data\n");
		return -ENODEV;
	}

	frame_len = (len / (fcnt * 2));
	for (i = 0; i < fcnt; i ++) {
		raw_ptr = raw + (index * frame_len);

		ili_dump_data(raw_ptr, 16, frame_len, ilits->xch_num, "recore_data");
		index--;
		if(index < 0)
			index = fcnt - 1;
	}

	if (!ice)
		ili_ice_mode_ctrl(DISABLE, ON);

	if (record_case == 2) {
		TS_LOG_INFO("tp_palm_stat = %d\n", record_stat.touch_palm_state_e);
		TS_LOG_INFO("app_an_stat = %d\n", record_stat.app_an_statu_e);
		TS_LOG_INFO("app_check_abnor = %d\n", record_stat.app_sys_check_bg_abnormal);
		TS_LOG_INFO("wrong_bg = %d\n", record_stat.g_b_wrong_bg);
	}

	ipio_kfree((void **)&raw);

	return 0;
}

void ili_gesture_fail_reason(bool enable)
{

	u8 cmd[24] = {0};

	/* set symbol */
	if (ili_ic_func_ctrl("knock_en", 0x8) < 0)
		TS_LOG_ERR("set symbol failed");

	/* enable gesture fail reason */
	cmd[0] = 0x01;
	cmd[1] = 0x0A;
	cmd[2] = 0x10;
	if (enable)
		cmd[3] = 0x01;
	else
		cmd[3] = 0x00;
	cmd[4] = 0xFF;
	cmd[5] = 0xFF;
	if ((ilits->wrapper(cmd, 6, NULL, 0, ON, OFF)) < 0)
		TS_LOG_ERR("enable gesture fail reason failed");

	/* set gesture parameters */
	cmd[0] = 0x01;
	cmd[1] = 0x0A;
	cmd[2] = 0x12;
	cmd[3] = 0x01;
	memset(cmd + 4, 0xFF, 20);
	if ((ilits->wrapper(cmd, 24, NULL, 0, ON, OFF)) < 0)
		TS_LOG_ERR("set gesture parameters failed");

	/* get gesture parameters */
	cmd[0] = 0x01;
	cmd[1] = 0x0A;
	cmd[2] = 0x11;
	cmd[3] = 0x01;
	if ((ilits->wrapper(cmd, 4, NULL, 0, ON, OFF)) < 0)
		TS_LOG_ERR("get gesture parameters failed");

}

int ili_tp_data_mode_ctrl(u8* cmd, int length)
{
	int ret = 0;
	switch (cmd[0]) {
	case AP_MODE:
		if (ilits->tp_suspend) {
			if (ili_set_tp_data_len(DATA_FORMAT_GESTURE_DEMO, false, NULL) < 0) {
				TS_LOG_ERR("Failed to set demo len from gesture mode\n");
				ret = -ENOTTY;
			}
		} else {
			if (ili_switch_tp_mode(P5_X_FW_AP_MODE) < 0) {
				TS_LOG_ERR("Failed to switch demo mode\n");
				ret = -ENOTTY;
			}
		}
		break;
	case TEST_MODE:
		if (ili_switch_tp_mode(P5_X_FW_TEST_MODE) < 0) {
			TS_LOG_ERR("Failed to switch test mode\n");
			ret = -ENOTTY;
		}
		break;
	case DEBUG_MODE:
		if (!ilits->tp_suspend && (ilits->actual_tp_mode != P5_X_FW_AP_MODE)) {
			if (ili_switch_tp_mode(P5_X_FW_AP_MODE) < 0) {
				TS_LOG_ERR("Failed to switch demo mode\n");
				ret = -ENOTTY;
				break;
			}
		}
		if (ilits->tp_suspend) {
			if (ili_set_tp_data_len(DATA_FORMAT_GESTURE_DEBUG, false, NULL) < 0) {
				TS_LOG_ERR("Failed to set debug len from gesture mode\n");
				ret = -ENOTTY;
			}
		} else {
			if (ili_set_tp_data_len(DATA_FORMAT_DEBUG, false, NULL) < 0) {
				TS_LOG_ERR("Failed to set debug len\n");
				ret = -ENOTTY;
			}
		}
		break;
	case DEBUG_LITE_ROI:
		if (ili_set_tp_data_len(DATA_FORMAT_DEBUG_LITE_ROI, false, NULL) < 0) {
			TS_LOG_ERR("Failed to set debug lite roi from gesture mode\n");
			ret = -ENOTTY;
		}
		break;
	case DEBUG_LITE_WINDOW:
		if (ili_set_tp_data_len(DATA_FORMAT_DEBUG_LITE_WINDOW, false, NULL) < 0) {
			TS_LOG_ERR("Failed to set debug lite window from gesture mode\n");
			ret = -ENOTTY;
		}
		break;
	case DEBUG_LITE_AREA:
		if (ili_set_tp_data_len(DATA_FORMAT_DEBUG_LITE_AREA, false, &cmd[1]) < 0) {
			TS_LOG_ERR("Failed to set debug lite area from gesture mode\n");
			ret = -ENOTTY;
		}
		break;
	default:
		TS_LOG_ERR("Unknown TP mode ctrl\n");
		ret = -ENOTTY;
		break;
	}

	return ret;
}

static ssize_t ilitek_node_ioctl_write(struct file *filp, const char *buff, size_t size, loff_t *pos)
{
	int i, ret = 0, count = 0;
	char cmd[512] = {0};
	char *token = NULL, *cur = NULL;
	u8 temp[256] = {0};
	u32 *data = NULL;

	if ((size - 1) > sizeof(cmd)) {
		TS_LOG_ERR("ERROR! input length is larger than local buffer\n");
		return -1;
	}

	mutex_lock(&ilits->touch_mutex);

	if (buff != NULL) {
		if (copy_from_user(cmd, buff, size - 1)) {
			TS_LOG_INFO("Failed to copy data from user space\n");
			return -1;
		}
	}

	TS_LOG_INFO("size = %d, cmd = %s\n", (int)size, cmd);

	token = cur = cmd;

	data = kcalloc(512, sizeof(u32), GFP_KERNEL);

	while ((token = strsep(&cur, ",")) != NULL) {
		data[count] = ili_str2hex(token);
		TS_LOG_INFO("data[%d] = %x\n", count, data[count]);
		count++;
	}

	TS_LOG_INFO("cmd = %s\n", cmd);

	if (strncmp(cmd, "hwreset", strlen(cmd)) == 0) {
		ili_reset_ctrl(TP_HW_RST_ONLY);
	} else if (strcmp(cmd, "rawdatarecore") == 0) {
		if (data[1] == ENABLE_RECORD)
			ili_get_tp_recore_ctrl(ENABLE_RECORD);
		else if (data[1] == DATA_RECORD)
			ili_get_tp_recore_ctrl(DATA_RECORD);
		else if (data[1] == DISABLE_RECORD)
			ili_get_tp_recore_ctrl(DISABLE_RECORD);
	} else if (strcmp(cmd, "switchdemodebuginfomode") == 0) {
		ili_set_tp_data_len(DATA_FORMAT_DEMO_DEBUG_INFO, false, NULL);
	} else if (strcmp(cmd, "gesturedemoen") == 0) {
		if (data[1] == DISABLE)
			ilits->gesture_demo_ctrl = DISABLE;
		else
			ilits->gesture_demo_ctrl = ENABLE;
		ili_set_tp_data_len(DATA_FORMAT_GESTURE_DEMO, false, NULL);
	} else if (strcmp(cmd, "gesturefailrsn") == 0) {
		if (data[1] == DISABLE)
			ili_gesture_fail_reason(DISABLE);
		else
			ili_gesture_fail_reason(ENABLE);
		TS_LOG_INFO("%s gesture fail reason\n", data[1] ? "ENABLE" : "DISABLE");
	} else if (strncmp(cmd, "icwholereset", strlen(cmd)) == 0) {
		ili_ice_mode_ctrl(ENABLE, OFF);
		ili_reset_ctrl(TP_IC_WHOLE_RST);
	} else if (strncmp(cmd, "iccodereset", strlen(cmd)) == 0) {
		ili_ice_mode_ctrl(ENABLE, OFF);
		ili_reset_ctrl(TP_IC_CODE_RST);
		ili_ice_mode_ctrl(DISABLE, OFF);
	} else if (strncmp(cmd, "enableirq", strlen(cmd)) == 0) {
		TS_LOG_INFO("Enable IRQ\n");
		ili_irq_enable();
	} else if (strncmp(cmd, "disableirq", strlen(cmd)) == 0) {
		TS_LOG_INFO("Disable IRQ\n");
		ili_irq_disable();
	} else if (strncmp(cmd, "infofromhex", strlen(cmd)) == 0) {
		ilits->info_from_hex = data[1];
		TS_LOG_INFO("info from hex = %d\n", data[1]);
	} else if (strncmp(cmd, "getinfo", strlen(cmd)) == 0) {
		ili_ice_mode_ctrl(ENABLE, OFF);
		ili_ic_get_info();
		ili_ice_mode_ctrl(DISABLE, OFF);
		ili_ic_get_protocl_ver();
		ili_ic_get_fw_ver();
		ili_ic_get_core_ver();
		ili_ic_get_tp_info();
		ili_ic_get_panel_info();
		TS_LOG_INFO("Driver version = %s\n", DRIVER_VERSION);
		TS_LOG_INFO("TP module = %s\n", ilits->md_name);
	} else if (strncmp(cmd, "enableicemode", strlen(cmd)) == 0) {
		if (data[1] == ON)
			ili_ice_mode_ctrl(ENABLE, ON);
		else
			ili_ice_mode_ctrl(ENABLE, OFF);
	} else if (strncmp(cmd, "wqctrl", strlen(cmd)) == 0) {
		ilits->wq_ctrl = !ilits->wq_ctrl;
		TS_LOG_INFO("wq_ctrl flag= %d\n", ilits->wq_ctrl);
	} else if (strncmp(cmd, "disableicemode", strlen(cmd)) == 0) {
		ili_ice_mode_ctrl(DISABLE, OFF);
	} else if (strncmp(cmd, "enablewqesd", strlen(cmd)) == 0) {
		ili_wq_ctrl(WQ_ESD, ENABLE);
	} else if (strncmp(cmd, "enablewqbat", strlen(cmd)) == 0) {
		ili_wq_ctrl(WQ_BAT, ENABLE);
	} else if (strncmp(cmd, "disablewqesd", strlen(cmd)) == 0) {
		ili_wq_ctrl(WQ_ESD, DISABLE);
	} else if (strncmp(cmd, "disablewqbat", strlen(cmd)) == 0) {
		ili_wq_ctrl(WQ_BAT, DISABLE);
	} else if (strncmp(cmd, "gesture", strlen(cmd)) == 0) {
		ilits->gesture = !ilits->gesture;
		TS_LOG_INFO("gesture = %d\n", ilits->gesture);
	} else if (strncmp(cmd, "esddebug", strlen(cmd)) == 0) {
		ilits->esd_debug= !ilits->esd_debug;
		TS_LOG_INFO("esddebug = %d\n", ilits->esd_debug);
	} else if (strncmp(cmd, "esddebugdelay", strlen(cmd)) == 0) {
		ilits->esd_debug_delay= data[1];
		TS_LOG_INFO("esddebugdelay = %d ms\n", ilits->esd_debug_delay);
	} else if (strncmp(cmd, "esdgesture", strlen(cmd)) == 0) {
		if (ili_gesture_recovery() < 0) {
			TS_LOG_ERR("Gesture recovery failed\n");
			size = -1;
		}
	} else if (strncmp(cmd, "esdspi", strlen(cmd)) == 0) {
		ili_spi_recovery();
	} else if (strncmp(cmd, "sleepin", strlen(cmd)) == 0) {
		ili_ic_func_ctrl("sleep", SLEEP_IN);
	} else if (strncmp(cmd, "deepsleepin", strlen(cmd)) == 0) {
		ili_ic_func_ctrl("sleep", DEEP_SLEEP_IN);
	} else if (strncmp(cmd, "iceflag", strlen(cmd)) == 0) {
		if (data[1] == ENABLE)
			atomic_set(&ilits->ice_stat, ENABLE);
		else
			atomic_set(&ilits->ice_stat, DISABLE);
		TS_LOG_INFO("ice mode flag = %d\n", atomic_read(&ilits->ice_stat));
	} else if (strncmp(cmd, "gesturenormal", strlen(cmd)) == 0) {
		ilits->gesture_mode = DATA_FORMAT_GESTURE_NORMAL;
		TS_LOG_INFO("gesture mode = %d\n", ilits->gesture_mode);
	} else if (strncmp(cmd, "gestureinfo", strlen(cmd)) == 0) {
		ilits->gesture_mode = DATA_FORMAT_GESTURE_INFO;
		TS_LOG_INFO("gesture mode = %d\n", ilits->gesture_mode);
	} else if (strncmp(cmd, "netlink", strlen(cmd)) == 0) {
		ilits->netlink = !ilits->netlink;
		TS_LOG_INFO("netlink flag= %d\n", ilits->netlink);
	} else if (strncmp(cmd, "switchtestmode", strlen(cmd)) == 0) {
		ili_switch_tp_mode(P5_X_FW_TEST_MODE);
	} else if (strncmp(cmd, "switchdebugmode", strlen(cmd)) == 0) {
		ili_set_tp_data_len(DATA_FORMAT_DEBUG, false, NULL);
	} else if (strncmp(cmd, "switchdemomode", strlen(cmd)) == 0) {
		ili_set_tp_data_len(DATA_FORMAT_DEMO, false, NULL);
	} else if (strncmp(cmd, "switchgesturedebugmode", strlen(cmd)) == 0) {
		ili_set_tp_data_len(DATA_FORMAT_GESTURE_DEBUG, false, NULL);
	} else if (strncmp(cmd, "dbgflag", strlen(cmd)) == 0) {
		ilits->dnp = !ilits->dnp;
		TS_LOG_INFO("debug flag message = %d\n", ilits->dnp);
	} else if (strncmp(cmd, "spiclk", strlen(cmd)) == 0) {
		TS_LOG_INFO("spi clk num = %d\n", data[1]);
		ili_core_spi_setup(data[1]);
	} else if (strncmp(cmd, "ss", strlen(cmd)) == 0) {
		TS_LOG_INFO("sense_stop = %d\n", data[1]);
		ilits->ss_ctrl = data[1];
	} else if (strncmp(cmd, "iow", strlen(cmd)) == 0) {
		int w_len = 0;
		w_len = data[1];
		TS_LOG_INFO("w_len = %d\n", w_len);
		for (i = 0; i < w_len; i++) {
			temp[i] = data[2 + i];
			TS_LOG_INFO("write[%d] = %x\n", i, temp[i]);
		}

		ilits->wrapper(temp, w_len, NULL, 0, ON, OFF);
	} else if (strncmp(cmd, "ior", strlen(cmd)) == 0) {
		int r_len = 0;
		r_len = data[1];
		TS_LOG_INFO("r_len = %d\n", r_len);
		ilits->wrapper(NULL, 0, temp, r_len, ON, OFF);
		for (i = 0; i < r_len; i++)
			TS_LOG_INFO("read[%d] = %x\n", i, temp[i]);
	} else if (strncmp(cmd, "iowr", strlen(cmd)) == 0) {
		int delay = 0;
		int w_len = 0, r_len = 0;
		w_len = data[1];
		r_len = data[2];
		delay = data[3];
		TS_LOG_INFO("w_len = %d, r_len = %d, delay = %d\n", w_len, r_len, delay);

		for (i = 0; i < w_len; i++) {
			temp[i] = data[4 + i];
			TS_LOG_INFO("write[%d] = %x\n", i, temp[i]);
		}
		ret = ilits->wrapper(temp, w_len, NULL, 0, OFF, OFF);
		memset(temp, 0, sizeof(temp));
		mdelay(delay);
		ilits->wrapper(NULL, 0, temp, r_len, OFF, OFF);
		for (i = 0; i < r_len; i++)
			TS_LOG_INFO("read[%d] = %x\n", i, temp[i]);
	} else if (strcmp(cmd, "iowrint") == 0) {
		int delay = 0;
		int w_len = 0, r_len = 0;
		w_len = data[1];
		r_len = data[2];
		delay = data[3];
		TS_LOG_INFO("w_len = %d, r_len = %d, delay = %d\n", w_len, r_len, delay);
		for (i = 0; i < w_len; i++) {
			temp[i] = data[4 + i];
			TS_LOG_INFO("write[%d] = %x\n", i, temp[i]);
		}
		ret = ilits->wrapper(temp, w_len, temp, r_len, ON, ON);
		for (i = 0; i < r_len; i++)
			TS_LOG_INFO("read[%d] = %x\n", i, temp[i]);
	} else if (strncmp(cmd, "getddiregdata", strlen(cmd)) == 0) {
		u8 ddi_data = 0;
		TS_LOG_INFO("Get ddi reg one page: page = %x, reg = %x\n", data[1], data[2]);
		ili_ic_get_ddi_reg_onepage(data[1], data[2], &ddi_data);
		TS_LOG_INFO("ddi_data = %x\n", ddi_data);
	} else if (strncmp(cmd, "setddiregdata", strlen(cmd)) == 0) {
		TS_LOG_INFO("Set ddi reg one page: page = %x, reg = %x, data = %x\n", data[1], data[2], data[3]);
		ili_ic_set_ddi_reg_onepage(data[1], data[2], data[3]);
	} else if (strncmp(cmd, "dumpflashdata", strlen(cmd)) == 0) {
		ilits->fw_update_stat = FW_STAT_INIT;
		TS_LOG_INFO("Start = 0x%x, End = 0x%x, Dump Hex path = %s\n", data[1], data[2], DUMP_FLASH_PATH);
		ret = ili_fw_dump_flash_data(data[1], data[2], false);
		ilits->fw_update_stat = (ret < 0) ? FW_UPDATE_FAIL : FW_UPDATE_PASS;
		TS_LOG_INFO("ilits->fw_update_stat = %d\n", ilits->fw_update_stat);
	} else if (strncmp(cmd, "dumpiramdata", strlen(cmd)) == 0) {
		TS_LOG_INFO("Start = 0x%x, End = 0x%x, Dump IRAM path = %s\n", data[1], data[2], DUMP_IRAM_PATH);
		ilits->fw_update_stat = FW_STAT_INIT;
		ret = ili_fw_dump_iram_data(data[1], data[2], true);
		ilits->fw_update_stat = (ret < 0) ? FW_UPDATE_FAIL : FW_UPDATE_PASS;
		TS_LOG_INFO("ilits->fw_update_stat = %d\n", ilits->fw_update_stat);
	} else if (strncmp(cmd, "edge_palm_ctrl", strlen(cmd)) == 0) {
		ili_ic_func_ctrl("edge_palm", data[1]);
	} else if (strncmp(cmd, "uart_mode_ctrl", strlen(cmd)) == 0) {
		ili_fw_uart_ctrl(data[1]);
	} else if (strncmp(cmd, "flashesdgesture", strlen(cmd)) == 0) {
		ili_touch_esd_gesture_flash();
	} else if (strncmp(cmd, "intact", strlen(cmd)) == 0) {
		TS_LOG_INFO("Set INT trigger = %x\n", data[1]);
		ili_ic_int_trigger_ctrl(data[1]);
	} else if (strncmp(cmd, "spiw", strlen(cmd)) == 0) {
		int wlen;
		wlen = data[1];
		temp[0] = 0x82;
		for (i = 0; i < wlen; i++) {
			temp[i] = data[2 + i];
			TS_LOG_INFO("write[%d] = %x\n", i, temp[i]);
		}
		ilits->spi_write_then_read(ilits->spi, temp, wlen, NULL, 0);
	} else if (strncmp(cmd, "spir", strlen(cmd)) == 0) {
		int rlen;
		u8 *rbuf = NULL;
		rlen = data[1];
		rbuf = kzalloc(rlen, GFP_KERNEL | GFP_DMA);
		if (err_alloc_mem(rbuf)) {
			TS_LOG_ERR("Failed to allocate dma_rxbuf, %ld\n", PTR_ERR(rbuf));
			kfree(rbuf);
			goto out;
		}
		temp[0] = 0x83;
		ilits->spi_write_then_read(ilits->spi, temp, 1, rbuf, rlen);
		for (i = 0; i < rlen; i++)
			TS_LOG_INFO("read[%d] = %x\n", i, rbuf[i]);
		kfree(rbuf);
	} else if (strncmp(cmd, "spirw", strlen(cmd)) == 0) {
		int wlen, rlen;
		u8 *rbuf = NULL;
		wlen = data[1];
		rlen = data[2];
		for (i = 0; i < wlen; i++) {
			temp[i] = data[3 + i];
			TS_LOG_INFO("write[%d] = %x\n", i, temp[i]);
		}
		if (rlen != 0) {
			rbuf = kzalloc(rlen, GFP_KERNEL | GFP_DMA);
			if (err_alloc_mem(rbuf)) {
				TS_LOG_ERR("Failed to allocate dma_rxbuf, %ld\n", PTR_ERR(rbuf));
				kfree(rbuf);
				goto out;
			}
		}
		ilits->spi_write_then_read(ilits->spi, temp, wlen, rbuf, rlen);
		if (rlen != 0) {
			for (i = 0; i < rlen; i++)
				TS_LOG_INFO("read[%d] = %x\n", i, rbuf[i]);
		}
		kfree(rbuf);
	} else if (strncmp(cmd, "ges_sym", strlen(cmd)) == 0) {
		struct gesture_symbol *ptr_sym = &ilits->ges_sym;
		u8 *ptr;
		ptr = (u8*) ptr_sym;
		ptr[0] = data[1];
		ptr[1] = data[2];
		ptr[2] = data[3];
		ili_dump_data(ptr, 8, 3, 0, "Gesture symbol");
	} else if (strncmp(cmd, "mode", strlen(cmd)) == 0) {
		temp[0] = data[1];
		temp[1] = data[2];
		temp[2] = data[3];
		temp[3] = data[4];
		temp[4] = data[5];
		ret = ili_tp_data_mode_ctrl(temp, sizeof(temp)/temp[0]);
	} else if (strncmp(cmd, "switchmpretry", strlen(cmd)) == 0) {
		ilits->mp_retry = !ilits->mp_retry;
		TS_LOG_INFO("switchmpretry = %d\n", ilits->mp_retry);
	} else {
		TS_LOG_ERR("Unknown command\n");
		size = -1;
	}
out:
	ipio_kfree((void **)&data);
	mutex_unlock(&ilits->touch_mutex);
	return size;
}

#ifdef CONFIG_COMPAT
static long ilitek_node_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	if (!filp->f_op || !filp->f_op->unlocked_ioctl) {
		TS_LOG_ERR("There's no unlocked_ioctl defined in file\n");
		return -ENOTTY;
	}

	TS_LOG_DEBUG("cmd = %d\n", _IOC_NR(cmd));

	switch (cmd) {
	case ILITEK_COMPAT_IOCTL_I2C_WRITE_DATA:
		TS_LOG_DEBUG("compat_ioctl: convert i2c/spi write\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_I2C_WRITE_DATA, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_I2C_READ_DATA:
		TS_LOG_DEBUG("compat_ioctl: convert i2c/spi read\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_I2C_READ_DATA, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_I2C_SET_WRITE_LENGTH:
		TS_LOG_DEBUG("compat_ioctl: convert set write length\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_I2C_SET_WRITE_LENGTH, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_I2C_SET_READ_LENGTH:
		TS_LOG_DEBUG("compat_ioctl: convert set read length\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_I2C_SET_READ_LENGTH, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_HW_RESET:
		TS_LOG_DEBUG("compat_ioctl: convert hw reset\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_HW_RESET, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_POWER_SWITCH:
		TS_LOG_DEBUG("compat_ioctl: convert power switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_POWER_SWITCH, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_REPORT_SWITCH:
		TS_LOG_DEBUG("compat_ioctl: convert report switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_REPORT_SWITCH, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_IRQ_SWITCH:
		TS_LOG_DEBUG("compat_ioctl: convert irq switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_IRQ_SWITCH, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_DEBUG_LEVEL:
		TS_LOG_DEBUG("compat_ioctl: convert debug level\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_DEBUG_LEVEL, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_FUNC_MODE:
		TS_LOG_DEBUG("compat_ioctl: convert format mode\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_FUNC_MODE, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_FW_VER:
		TS_LOG_DEBUG("compat_ioctl: convert set read length\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_FW_VER, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_PL_VER:
		TS_LOG_DEBUG("compat_ioctl: convert fw version\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_PL_VER, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_CORE_VER:
		TS_LOG_DEBUG("compat_ioctl: convert core version\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_CORE_VER, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_DRV_VER:
		TS_LOG_DEBUG("compat_ioctl: convert driver version\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_DRV_VER, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_CHIP_ID:
		TS_LOG_DEBUG("compat_ioctl: convert chip id\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_CHIP_ID, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_NETLINK_CTRL:
		TS_LOG_DEBUG("compat_ioctl: convert netlink ctrl\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_NETLINK_CTRL, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_NETLINK_STATUS:
		TS_LOG_DEBUG("compat_ioctl: convert netlink status\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_NETLINK_STATUS, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_MODE_CTRL:
		TS_LOG_DEBUG("compat_ioctl: convert tp mode ctrl\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_MODE_CTRL, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_MODE_STATUS:
		TS_LOG_DEBUG("compat_ioctl: convert tp mode status\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_MODE_STATUS, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_ICE_MODE_SWITCH:
		TS_LOG_DEBUG("compat_ioctl: convert tp mode switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_ICE_MODE_SWITCH, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_INTERFACE_TYPE:
		TS_LOG_DEBUG("compat_ioctl: convert interface type\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_INTERFACE_TYPE, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_DUMP_FLASH:
		TS_LOG_DEBUG("compat_ioctl: convert dump flash\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_DUMP_FLASH, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_FW_UART_CTRL:
		TS_LOG_DEBUG("compat_ioctl: convert fw uart\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_IOCTL_TP_FW_UART_CTRL, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_PANEL_INFO:
		TS_LOG_DEBUG("compat_ioctl: convert resolution\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_COMPAT_IOCTL_TP_PANEL_INFO, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_TP_INFO:
		TS_LOG_DEBUG("compat_ioctl: convert tp info\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_COMPAT_IOCTL_TP_INFO, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_WRAPPER_RW:
		TS_LOG_DEBUG("compat_ioctl: convert wrapper\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_COMPAT_IOCTL_WRAPPER_RW, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_DDI_WRITE:
		TS_LOG_DEBUG("compat_ioctl: convert ddi write\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_COMPAT_IOCTL_DDI_WRITE, (uintptr_t)compat_ptr(arg));
		return ret;
	case ILITEK_COMPAT_IOCTL_DDI_READ:
		TS_LOG_DEBUG("compat_ioctl: convert ddi read\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			ILITEK_COMPAT_IOCTL_DDI_READ, (uintptr_t)compat_ptr(arg));
		return ret;
	default:
		TS_LOG_ERR("no ioctl cmd, return ilitek_node_ioctl\n");
		return -ENOIOCTLCMD;
	}
}
#endif

static long ilitek_node_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0, length = 0;
	u8 *szbuf = NULL, if_to_user = 0;
	static u16 i2c_rw_length;
	u32 id_to_user[64] = {0};
	bool esd_en = ilits->wq_esd_ctrl, bat_en = ilits->wq_bat_ctrl;
	u8 *wrap_rbuf = NULL, wrap_int = 0;
	u16 wrap_wlen = 0, wrap_rlen = 0;
	bool spi_irq = OFF, i2c_irq = OFF;

	if (_IOC_TYPE(cmd) != ILITEK_IOCTL_MAGIC) {
		TS_LOG_ERR("The Magic number doesn't match\n");
		return -ENOTTY;
	}

	if (_IOC_NR(cmd) > ILITEK_IOCTL_MAXNR) {
		TS_LOG_ERR("The number of ioctl doesn't match\n");
		return -ENOTTY;
	}

	TS_LOG_DEBUG("cmd = %d\n", _IOC_NR(cmd));

	mutex_lock(&ilits->touch_mutex);

	szbuf = kcalloc(IOCTL_I2C_BUFF, sizeof(u8), GFP_KERNEL);
	if (err_alloc_mem(szbuf)) {
		TS_LOG_ERR("Failed to allocate mem\n");
		ret = -ENOMEM;
		goto out;
	}

	if (esd_en)
		ili_wq_ctrl(WQ_ESD, DISABLE);
	if (bat_en)
		ili_wq_ctrl(WQ_BAT, DISABLE);

	switch (cmd) {
	case ILITEK_IOCTL_I2C_WRITE_DATA:
		TS_LOG_DEBUG("ioctl: write len = %d\n", i2c_rw_length);

		if (i2c_rw_length > IOCTL_I2C_BUFF) {
			TS_LOG_ERR("ERROR! write len is largn than ioctl buf %d, %ld\n",
					i2c_rw_length, IOCTL_I2C_BUFF);
			ret = -ENOTTY;
			break;
		}

		if (copy_from_user(szbuf, (u8 *)(uintptr_t) arg, i2c_rw_length)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}

		ret = ilits->wrapper(szbuf, i2c_rw_length, NULL, 0, OFF, OFF);
		if (ret < 0)
			TS_LOG_ERR("Failed to write data\n");
		break;
	case ILITEK_IOCTL_I2C_READ_DATA:
		TS_LOG_DEBUG("ioctl: read len = %d\n", i2c_rw_length);

		if (i2c_rw_length > IOCTL_I2C_BUFF) {
			TS_LOG_ERR("ERROR! read len is largn than ioctl buf %d, %ld\n",
					i2c_rw_length, IOCTL_I2C_BUFF);
			ret = -ENOTTY;
			break;
		}

		ret = ilits->wrapper(NULL, 0, szbuf, i2c_rw_length, OFF, OFF);
		if (ret < 0) {
			TS_LOG_ERR("Failed to read data\n");
			break;
		}

		if (copy_to_user((u8 *)(uintptr_t) arg, szbuf, i2c_rw_length)) {
			TS_LOG_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_I2C_SET_WRITE_LENGTH:
	case ILITEK_IOCTL_I2C_SET_READ_LENGTH:
		i2c_rw_length = arg;
		break;
	case ILITEK_IOCTL_TP_HW_RESET:
		TS_LOG_DEBUG("ioctl: hw reset\n");
		ili_reset_ctrl(ilits->reset);
		break;
	case ILITEK_IOCTL_TP_POWER_SWITCH:
		TS_LOG_INFO("Not implemented yet\n");
		break;
	case ILITEK_IOCTL_TP_REPORT_SWITCH:
		if (copy_from_user(szbuf, (u8 *)(uintptr_t) arg, 1)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}

		TS_LOG_DEBUG("ioctl: report switch = %d\n", szbuf[0]);
		if (szbuf[0]) {
			ilits->report = ENABLE;
			TS_LOG_DEBUG("report is enabled\n");
		} else {
			ilits->report = DISABLE;
			TS_LOG_DEBUG("report is disabled\n");
		}
		break;
	case ILITEK_IOCTL_TP_IRQ_SWITCH:
		if (copy_from_user(szbuf, (u8 *)(uintptr_t) arg, 1)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		TS_LOG_DEBUG("ioctl: irq switch = %d\n", szbuf[0]);
		if (szbuf[0])
			ili_irq_enable();
		else
			ili_irq_disable();
		break;
	case ILITEK_IOCTL_TP_DEBUG_LEVEL:
		if (copy_from_user(szbuf, (u8 *)(uintptr_t) arg, sizeof(u32))) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}

		debug_en = !debug_en;
		TS_LOG_DEBUG("debug_en = %d", debug_en);
		break;
	case ILITEK_IOCTL_TP_FUNC_MODE:
		if (copy_from_user(szbuf, (u8 *)(uintptr_t) arg, 3)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}

		TS_LOG_DEBUG("ioctl: set func mode = %x,%x,%x\n", szbuf[0], szbuf[1], szbuf[2]);
		ret = ilits->wrapper(szbuf, 3, NULL, 0, ON, OFF);
		break;
	case ILITEK_IOCTL_TP_FW_VER:
		TS_LOG_DEBUG("ioctl: get fw and mp version\n");
		szbuf[7] = ilits->chip->fw_mp_ver & 0xFF;
		szbuf[6] = (ilits->chip->fw_mp_ver >> 8) & 0xFF;
		szbuf[5] = (ilits->chip->fw_mp_ver >> 16) & 0xFF;
		szbuf[4] = ilits->chip->fw_mp_ver >> 24;
		szbuf[3] = ilits->chip->fw_ver & 0xFF;
		szbuf[2] = (ilits->chip->fw_ver >> 8) & 0xFF;
		szbuf[1] = (ilits->chip->fw_ver >> 16) & 0xFF;
		szbuf[0] = ilits->chip->fw_ver >> 24;
		TS_LOG_DEBUG("Firmware version = %d.%d.%d.%d\n", szbuf[0], szbuf[1], szbuf[2], szbuf[3]);
		TS_LOG_DEBUG("Firmware MP version = %d.%d.%d.%d\n", szbuf[4], szbuf[5], szbuf[6], szbuf[7]);

		if (copy_to_user((u8 *)(uintptr_t) arg, szbuf, 8)) {
			TS_LOG_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_TP_PL_VER:
		TS_LOG_DEBUG("ioctl: get protocl version\n");
		szbuf[2] = ilits->protocol->ver & 0xFF;
		szbuf[1] = (ilits->protocol->ver >> 8) & 0xFF;
		szbuf[0] = ilits->protocol->ver >> 16;
		TS_LOG_DEBUG("Protocol version = %d.%d.%d\n", szbuf[0], szbuf[1], szbuf[2]);

		if (copy_to_user((u8 *)(uintptr_t) arg, szbuf, 3)) {
			TS_LOG_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_TP_CORE_VER:
		TS_LOG_DEBUG("ioctl: get core version\n");
		if (ilits->protocol->core_ver_len == P5_X_CORE_VER_THREE_LENGTH)
			szbuf[3] = 0xFF;
		else
			szbuf[3] = ilits->chip->core_ver & 0xFF;
		szbuf[2] = (ilits->chip->core_ver >> 8) & 0xFF;
		szbuf[1] = (ilits->chip->core_ver >> 16) & 0xFF;
		szbuf[0] = ilits->chip->core_ver >> 24;
		TS_LOG_DEBUG("Core version = %d.%d.%d.%d, length = %d\n",
			szbuf[0], szbuf[1], szbuf[2], szbuf[3], ilits->protocol->core_ver_len);

		if (copy_to_user((u8 *)(uintptr_t) arg, szbuf, 4)) {
			TS_LOG_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_TP_DRV_VER:
		TS_LOG_DEBUG("ioctl: get driver version\n");
		length = snprintf(szbuf, USER_STR_BUFF * sizeof(unsigned char), "%s", DRIVER_VERSION);

		if (copy_to_user((u8 *)(uintptr_t) arg, szbuf, length)) {
			TS_LOG_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_TP_CHIP_ID:
		TS_LOG_DEBUG("ioctl: get chip id\n");
		id_to_user[0] = ilits->chip->pid;
		id_to_user[1] = ilits->chip->otp_id;
		id_to_user[2] = ilits->chip->ana_id;

		if (copy_to_user((u32 *)(uintptr_t) arg, id_to_user, sizeof(u32) * 3)) {
			TS_LOG_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_TP_NETLINK_CTRL:
		if (copy_from_user(szbuf, (u8 *)(uintptr_t) arg, 1)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		TS_LOG_DEBUG("ioctl: netlink ctrl = %d\n", szbuf[0]);
		if (szbuf[0]) {
			ilits->netlink = ENABLE;
			TS_LOG_DEBUG("ioctl: Netlink is enabled\n");
		} else {
			ilits->netlink = DISABLE;
			TS_LOG_DEBUG("ioctl: Netlink is disabled\n");
		}
		break;
	case ILITEK_IOCTL_TP_NETLINK_STATUS:
		TS_LOG_DEBUG("ioctl: get netlink stat = %d\n", ilits->netlink);
		if (copy_to_user((int *)(uintptr_t)arg, &ilits->netlink, sizeof(int))) {
			TS_LOG_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_TP_MODE_CTRL:
		if (copy_from_user(szbuf, (u8 *)(uintptr_t) arg, 10)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		TS_LOG_DEBUG("ioctl: switch fw format = %d\n", szbuf[0]);
		ret = ili_tp_data_mode_ctrl(szbuf, sizeof(u8));

		break;
	case ILITEK_IOCTL_TP_MODE_STATUS:
		TS_LOG_DEBUG("ioctl: current firmware mode = %d", ilits->actual_tp_mode);
		if (copy_to_user((int *)(uintptr_t)arg, &ilits->actual_tp_mode, sizeof(int))) {
			TS_LOG_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;
	/* It works for host downloado only */
	case ILITEK_IOCTL_ICE_MODE_SWITCH:
		if (copy_from_user(szbuf, (u8 *)(uintptr_t) arg, 1)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}

		TS_LOG_DEBUG("ioctl: switch ice mode = %d, mcu on = %d\n", szbuf[0], szbuf[1]);

		if (ili_ice_mode_ctrl(szbuf[0] ? ENABLE : DISABLE,
			szbuf[1] ? ENABLE : DISABLE) < 0)
			TS_LOG_ERR("ioctl: set ice mode failed\n");

		break;
	case ILITEK_IOCTL_TP_INTERFACE_TYPE:
		if_to_user = ilits->hwif->bus_type;
		if (copy_to_user((u8 *)(uintptr_t) arg, &if_to_user, sizeof(if_to_user))) {
			TS_LOG_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_TP_DUMP_FLASH:
		TS_LOG_DEBUG("ioctl: dump flash data\n");
		ret = ili_fw_dump_flash_data(0, 0, true);
		if (ret < 0)
			TS_LOG_ERR("ioctl: Failed to dump flash data\n");
		break;
	case ILITEK_IOCTL_TP_FW_UART_CTRL:
		if (copy_from_user(szbuf, (u8 *)(uintptr_t) arg, 1)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		TS_LOG_DEBUG("ioctl: fw UART  = %d\n", szbuf[0]);

		ili_fw_uart_ctrl(szbuf[0]);

		if_to_user = szbuf[0];

		if (copy_to_user((u8 *)(uintptr_t) arg, &if_to_user, sizeof(if_to_user))) {
			TS_LOG_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_TP_PANEL_INFO:
		TS_LOG_DEBUG("ioctl: get panel resolution\n");
		id_to_user[0] = ilits->panel_wid;
		id_to_user[1] = ilits->panel_hei;

		if (copy_to_user((u32 *)(uintptr_t) arg, id_to_user, sizeof(u32) * 2)) {
			TS_LOG_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_TP_INFO:
		TS_LOG_DEBUG("ioctl: get tp info\n");
		id_to_user[0] = ilits->min_x;
		id_to_user[1] = ilits->min_y;
		id_to_user[2] = ilits->max_x;
		id_to_user[3] = ilits->max_y;
		id_to_user[4] = ilits->xch_num;
		id_to_user[5] = ilits->ych_num;
		id_to_user[6] = ilits->stx;
		id_to_user[7] = ilits->srx;

		if (copy_to_user((u32 *)(uintptr_t) arg, id_to_user, sizeof(u32) * 8)) {
			TS_LOG_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_WRAPPER_RW:
		TS_LOG_DEBUG("ioctl: wrapper rw\n");

		if (i2c_rw_length > IOCTL_I2C_BUFF || i2c_rw_length == 0) {
			TS_LOG_ERR("ERROR! i2c_rw_length is is invalid\n");
			ret = -ENOTTY;
			break;
		}

		if (copy_from_user(szbuf, (u8 *)(uintptr_t) arg, i2c_rw_length)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}

		wrap_int = szbuf[0];
		wrap_rlen = (szbuf[1] << 8) | szbuf[2];
		wrap_wlen = (szbuf[3] << 8) | szbuf[4];

		TS_LOG_DEBUG("wrap_int = %d\n", wrap_int);
		TS_LOG_DEBUG("wrap_rlen = %d\n", wrap_rlen);
		TS_LOG_DEBUG("wrap_wlen = %d\n", wrap_wlen);

		if (wrap_wlen > IOCTL_I2C_BUFF || wrap_rlen > IOCTL_I2C_BUFF) {
			TS_LOG_ERR("ERROR! R/W len is largn than ioctl buf\n");
			ret = -ENOTTY;
			break;
		}

		if (wrap_rlen > 0) {
			wrap_rbuf = kcalloc(IOCTL_I2C_BUFF, sizeof(u8), GFP_KERNEL);
			if (err_alloc_mem(wrap_rbuf)) {
				TS_LOG_ERR("Failed to allocate mem\n");
				ret = -ENOMEM;
				break;
			}
		}

		if (wrap_int == 1) {
			i2c_irq = ON;
			spi_irq = ON;
		} else if (wrap_int == 2) {
			i2c_irq = OFF;
			spi_irq = OFF;
		} else {
			i2c_irq= OFF;
			spi_irq = (wrap_rlen > 0 ? ON : OFF);
		}

		TS_LOG_DEBUG("i2c_irq = %d, spi_irq = %d\n", i2c_irq, spi_irq);

		ilits->wrapper(szbuf + 5, wrap_wlen, wrap_rbuf, wrap_rlen, spi_irq, i2c_irq);

		ili_dump_data(szbuf + 5, 8, wrap_wlen, 16, "wrap_wbuf:");
		ili_dump_data(wrap_rbuf, 8, wrap_rlen, 16, "wrap_rbuf:");
		if (copy_to_user((u8 *)(uintptr_t) arg, wrap_rbuf, wrap_rlen)) {
			TS_LOG_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;
	case ILITEK_IOCTL_DDI_WRITE:
		if (copy_from_user(szbuf, (u8 *)(uintptr_t)arg, 3)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		TS_LOG_DEBUG("ioctl: page = %x, reg = %x, data = %x\n", szbuf[0], szbuf[1], szbuf[2]);
		ili_ic_set_ddi_reg_onepage(szbuf[0], szbuf[1], szbuf[2]);
		break;
	case ILITEK_IOCTL_DDI_READ:
		if (copy_from_user(szbuf, (u8 *)(uintptr_t)arg, 2)) {
			TS_LOG_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		TS_LOG_DEBUG("ioctl: page = %x, reg = %x\n", szbuf[0], szbuf[1]);
		ili_ic_get_ddi_reg_onepage(szbuf[0], szbuf[1], &szbuf[2]);
		TS_LOG_DEBUG("ioctl: data = %x\n", szbuf[2]);
		if (copy_to_user((u8 *)(uintptr_t)arg, szbuf + 2, 1)) {
			TS_LOG_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;
	default:
		ret = -ENOTTY;
		break;
	}

	ipio_kfree((void **)&szbuf);
	ipio_kfree((void **)&wrap_rbuf);

out:
	if (esd_en)
		ili_wq_ctrl(WQ_ESD, ENABLE);
	if (bat_en)
		ili_wq_ctrl(WQ_BAT, ENABLE);

	mutex_unlock(&ilits->touch_mutex);
	return ret;
}

static struct proc_dir_entry *proc_dir_ilitek;

typedef struct {
	char *name;
	struct proc_dir_entry *node;
	struct file_operations *fops;
	bool iscreated;
} proc_node;

static struct file_operations proc_ver_info_fops = {
	.read = ilitek_node_ver_info_read,
};

static struct file_operations proc_change_list_fops = {
	.read = ilitek_node_change_list_read,
};

static struct file_operations proc_debug_message_fops = {
	.read = ilitek_proc_debug_message_read,
};

static struct file_operations proc_debug_message_switch_fops = {
	.read = ilitek_proc_debug_switch_read,
};

static struct file_operations proc_ioctl_fops = {
	.unlocked_ioctl = ilitek_node_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = ilitek_node_compat_ioctl,
#endif
	.write = ilitek_node_ioctl_write,
};

static struct file_operations proc_rw_tp_reg_fops = {
	.read = ilitek_proc_rw_tp_reg_read,
	.write = ilitek_proc_rw_tp_reg_write,
};

static struct file_operations proc_fw_pc_counter_fops = {
	.read = ilitek_proc_fw_pc_counter_read,
};

proc_node iliproc[] = {
	{"ioctl", NULL, &proc_ioctl_fops, false},
	{"debug_message", NULL, &proc_debug_message_fops, false},
	{"debug_message_switch", NULL, &proc_debug_message_switch_fops, false},
	{"fw_pc_counter", NULL, &proc_fw_pc_counter_fops, false},
	{"rw_tp_reg", NULL, &proc_rw_tp_reg_fops, false},
	{"ver_info", NULL, &proc_ver_info_fops, false},
	{"change_list", NULL, &proc_change_list_fops, false},
};

#define NETLINK_USER 21
static struct sock *netlink_skb;
static struct nlmsghdr *netlink_head;
static struct sk_buff *skb_out;
static int netlink_pid;

void ili_netlink_reply_msg(void *raw, int size)
{
	int ret;
	int msg_size = size;
	u8 *data = (u8 *) raw;

	TS_LOG_INFO("The size of data being sent to user = %d\n", msg_size);
	TS_LOG_INFO("pid = %d\n", netlink_pid);
	TS_LOG_INFO("Netlink is enable = %d\n", ilits->netlink);

	if (ilits->netlink) {
		skb_out = nlmsg_new(msg_size, 0);

		if (!skb_out) {
			TS_LOG_ERR("Failed to allocate new skb\n");
			return;
		}

		netlink_head = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
		NETLINK_CB(skb_out).dst_group = 0;	/* not in mcast group */

		if (msg_size > size)
			memcpy(nlmsg_data(netlink_head), data, size);
		else
			memcpy(nlmsg_data(netlink_head), data, msg_size);

		ret = nlmsg_unicast(netlink_skb, skb_out, netlink_pid);
		if (ret < 0)
			TS_LOG_ERR("Failed to send data back to user\n");
	}
}

static void netlink_recv_msg(struct sk_buff *skb)
{
	netlink_pid = 0;

	TS_LOG_INFO("Netlink = %d\n", ilits->netlink);

	netlink_head = (struct nlmsghdr *)skb->data;

	TS_LOG_INFO("Received a request from client: %s, %d\n",
		(char *)NLMSG_DATA(netlink_head), (int)strlen((char *)NLMSG_DATA(netlink_head)));

	/* pid of sending process */
	netlink_pid = netlink_head->nlmsg_pid;

	TS_LOG_INFO("the pid of sending process = %d\n", netlink_pid);

	/* TODO: may do something if there's not receiving msg from user. */
	if (netlink_pid != 0) {
		TS_LOG_ERR("The channel of Netlink has been established successfully !\n");
		ilits->netlink = ENABLE;
	} else {
		TS_LOG_ERR("Failed to establish the channel between kernel and user space\n");
		ilits->netlink = DISABLE;
	}
}

static int netlink_init(void)
{
	int ret = 0;

#if KERNEL_VERSION(3, 4, 0) > LINUX_VERSION_CODE
	netlink_skb = netlink_kernel_create(&init_net, NETLINK_USER, netlink_recv_msg, NULL, THIS_MODULE);
#else
	struct netlink_kernel_cfg cfg = {
		.input = netlink_recv_msg,
	};

	netlink_skb = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
#endif

	TS_LOG_INFO("Initialise Netlink and create its socket\n");

	if (!netlink_skb) {
		TS_LOG_ERR("Failed to create nelink socket\n");
		ret = -EFAULT;
	}
	return ret;
}

void ili_node_init(void)
{
	int i = 0, ret = 0;

	proc_dir_ilitek = proc_mkdir("ilitek", NULL);

	for (; i < ARRAY_SIZE(iliproc); i++) {
		iliproc[i].node = proc_create(iliproc[i].name, 0644, proc_dir_ilitek, iliproc[i].fops);

		if (iliproc[i].node == NULL) {
			iliproc[i].iscreated = false;
			TS_LOG_ERR("Failed to create %s under /proc\n", iliproc[i].name);
			ret = -ENODEV;
		} else {
			iliproc[i].iscreated = true;
			TS_LOG_INFO("Succeed to create %s under /proc\n", iliproc[i].name);
		}
	}
	netlink_init();
	ilits->esd_debug = 0;
	ilits->esd_debug_delay = 0;
}
