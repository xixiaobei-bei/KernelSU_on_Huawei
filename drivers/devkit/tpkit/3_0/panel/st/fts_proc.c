/*
 * fts_proc.c
 *
 * STMicroelectronics  marco.cali@st.com
 *
 * brief contains the function and variables needed to publish a file node in
 * the file system which allow to communicate with the IC from userspace
 *
 * Copyright (c) 2012-2019 STMicroelectronics marco.cali@st.com
 * Technologies Co., Ltd
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details
 */
#include "fts.h"
#include "fts_lib/ftsCompensation.h"
#include "fts_lib/ftsCore.h"
#include "fts_lib/ftsIO.h"
#include "fts_lib/ftsError.h"
#include "fts_lib/ftsFrame.h"
#include "fts_lib/ftsFlash.h"
#include "fts_lib/ftsTest.h"
#include "fts_lib/ftsTime.h"
#include "fts_lib/ftsTool.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#define DRIVER_DEBUG_FILE_NODE "driver_test" /* name of file node published */
/* Max chunk of data printed on the sequential file in each iteration */
#define CHUNK_PROC 1024
/* number of frames readingterations during the diagnostic test */
#define DIAGNOSTIC_NUM_FRAME 10

/* GUI utils byte ver */
#define CMD_READ_BYTE 0xF0 /* Byte output version of I2C/SPI read */
/* Byte output version of I2C/SPI write @see CMD_WRITE */
#define CMD_WRITE_BYTE 0xF1
/* Byte output version of I2C/SPI writeRead @see CMD_WRITEREAD */
#define CMD_WRITE_READ_BYTE 0xF2

/*
 * Byte output version of I2C/SPIwrite then writeRead
 * @see CMD_WRITE_THEN_WRITE_READ_BYTE
 */
#define CMD_WRITE_THEN_WRITE_READ_BYTE 0xF3
/* Byte output version of I2C/SPI writeU8UX @see CMD_WRITE_U8UX_BYTE */
#define CMD_WRITE_U8UX_BYTE 0xF4
/* Byte output version of I2C/SPI writeReadU8UX */
#define CMD_WRITE_READ_U8UX_BYTE 0xF5

/*
 * Byte output version of I2C/SPI writeU8UX then writeU8UX @see
 * CMD_WRITE_U8UX_THEN_WRITE_U8UX_BYTE
 */
#define CMD_WRITE_U8UX_THEN_WRITE_U8UX_BYTE 0xF6

/*
 * Byte output version of I2C/SPI writeU8UX  then writeReadU8UX @see
 * CMD_WRITE_U8UX_THEN_WRITE_READ_BYTE
 */
#define CMD_WRITE_U8UX_THEN_WRITE_READ_BYTE 0xF7
/* Byte output version of Production Limits File @see CMD_GETLIMITSFILE */
#define CMD_GET_LIMITS_FILE_BYTE 0xF8
/* Byte output version of FW file need to pass: @see CMD_GETFWFILE */
#define CMD_GET_FW_FILE_BYTE 0xF9
/* Byte output version of driver version and setting@see CMD_VERSION */
#define CMD_VERSION_BYTE 0xFA

#define DRIER_TEST_BUFF_MAX 15

/*
 * @defgroup scriptless Scriptless Protocol
 * @ingroup proc_file_code
 * Scriptless Protocol allows ST Software (such as FingerTip Studio etc) to
 * communicate with the IC from an user space.
 * This mode gives access to common bus operations (write, read etc) and
 * support additional functionalities. \n
 * The protocol is based on exchange of binary messages included between a
 * start and an end byte
 */

/* start byte of each message transferred in Scriptless Mode */
#define MESSAGE_START_BYTE 0x7B
/* end byte of each message transferred in Scriptless Mode */
#define MESSAGE_END_BYTE 0x7D
/*
 * minimun number of bytes of the structure of a messages exchanged
 * with host (include start/end byte, counter, actions, msg_size)
 */
#define MESSAGE_MIN_HEADER_SIZE 8
#define CMD_BUF_SIZE 7
#define CMD_WRITE_READ_PARAM 5
#define CMD_PARAM_TEMP 4
#define CMD_WRITE_PARAM 2
#define CMD_READ_PARAM 3
#define CMD_WRITE_THEN_WRITE_READ_PARAM 6
#define CMD_WRITE_U8UX_PARAM 4
#define CMD_WRITE_READ_PARAM_U8UX 6
#define CMD_WRITE_THEN_READ_PARAM 8
#define GET_FW_FILE_PARAM 3
#define BUFF_LENGTH 5
#define DRIVER_TEST_INITIAL_SIZE 6
#define BUFF_LEN 3
#define BUFF_END_LEN 2
#define PROC_LEN_MAX 8
#define FUNC_TEST_LEN 3
#define DEIVER_COUNT_SIZE 800
#define BYTE_SIZE 4


/* Possible actions that can be requested by an host */
typedef enum {
	ACTION_WRITE = (u16) 0x0001, /* Bus Write */
	ACTION_READ = (u16) 0x0002, /* Bus Read */
	ACTION_WRITE_READ = (u16) 0x0003, /* Bus Write followed by a Read */
	/* Get Version of the protocol
	 * (equal to the first 2 bye of driver version)
	 */
	ACTION_GET_VERSION = (u16) 0x0004,
	/* Bus Write with support to different address size */
	ACTION_WRITE_U8UX = (u16) 0x0011,
	/* Bus writeRead with support to different address size */
	ACTION_WRITE_READ_U8UX = (u16) 0x0012,
	/* Bus write followed by a writeRead */
	ACTION_WRITE_THEN_WRITE_READ = (u16) 0x0013,
	/* Bus write followed by a writeRead with support
	 * to different address size
	 */
	ACTION_WRITE_U8UX_THEN_WRITE_READ_U8UX = (u16) 0x0014,
	/* Bus write followed by a write with support to different
	 * address size
	 */
	ACTION_WRITE_U8UX_THEN_WRITE_U8UX = (u16) 0x0015,
	/* Get Fw file content used by the driver */
	ACTION_GET_FW = (u16) 0x1000,
	/* Get Limit File content used by the driver */
	ACTION_GET_LIMIT = (u16) 0x1001,
} Actions;

typedef struct {
	u16 msg_size; /* total size of the message in bytes */
	u16 counter; /* counter ID to identify a message */
	/* type of operation requested by the host @see Actions */
	Actions action;
	/*
	 * (optional)in case of any kind of read operations,
	 * specify if the first byte is dummy
	 */
	u8 dummy;
} Message;

/*
 * store the amount of data to print into the shell
 * @limit stores the memory allocation size of driver_test_buff
 */
static int limit;
/* store the chuk of data that should be printed in this iteration */
static int chunk;
static int printed; /* store the amount of data already printed in the shell */
/* reference to the directory fts under /proc */
static struct proc_dir_entry *fts_dir = NULL;

/*
 * pointer to an array of bytes used to store the result
 * of the function executed
 */
static u8 *driver_test_buff = NULL;

/* buffer used to store the message info received */
char buf_chunk[CHUNK_PROC];
/* store the information of the Scriptless message received */
static Message mess;
static struct mutex proc_touch_lock;
static DEFINE_MUTEX(proc_touch_lock);

static void *fts_seq_start(struct seq_file *s, loff_t *pos)
{
	if (!pos) {
		TS_LOG_ERR("%s:pos null\n", __func__);
		return NULL;
	}
	mutex_lock(&proc_touch_lock);
	TS_LOG_INFO("%s: ++ pos = %lld limit = %d printed = %d chunk %d\n",
		__func__, *pos, limit, printed, chunk);

	if ((driver_test_buff == NULL) && (*pos == 0)) {
		TS_LOG_ERR("%s: no data to print\n", __func__);
		driver_test_buff = kzalloc(DRIER_TEST_BUFF_MAX *
			sizeof(u8), GFP_KERNEL);
		if (driver_test_buff == NULL) {
			TS_LOG_ERR("%s driver_test_buff kmalloc failed!\n",
				__func__);
			mutex_unlock(&proc_touch_lock);
			return NULL;
		}
		snprintf(driver_test_buff, DRIER_TEST_BUFF_MAX - 1,
			"{ %08X }\n", ERROR_OP_NOT_ALLOW);
		limit = strlen(driver_test_buff);
	} else {
		TS_LOG_INFO("%s:pos = %lld\n", __func__, *pos);
		if (*pos != 0)
			*pos += chunk - 1;
		if (*pos >= limit) {
			mutex_unlock(&proc_touch_lock);
			return NULL;
		}
	}

	chunk = CHUNK_PROC;
	if ((limit - *pos) < CHUNK_PROC)
		chunk = limit - *pos;

	memset(buf_chunk, 0, CHUNK_PROC);
	memcpy(buf_chunk, &driver_test_buff[(int)*pos], chunk);
	mutex_unlock(&proc_touch_lock);
	return buf_chunk;
}

static int fts_seq_show(struct seq_file *s, void *v)
{
	TS_LOG_INFO("%s in show\n", __func__);
	if (!v) {
		TS_LOG_ERR("%s v data null\n", __func__);
		return ERROR_ALLOC;
	}
	mutex_lock(&proc_touch_lock);
	seq_write(s, (u8 *)v, chunk);
	printed += chunk;
	mutex_unlock(&proc_touch_lock);
	return 0;
}

/*
 * This function update the pointer and the counters to the next data to be
 * printed
 * @param s pointer to the sequential file where to print the data
 * @param v pointer to the data to print
 * @param pos pointer to the offset where write the next data
 * @return NULL if there is no data to print or the pointer to the beginning
 * of the next data that need to be printed
 */
static void *fts_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	TS_LOG_INFO("%s in next\n", __func__);
	if (!pos) {
		TS_LOG_ERR("%s:pos null\n", __func__);
		return NULL;
	}
	mutex_lock(&proc_touch_lock);
	TS_LOG_INFO("%s: ++ pos = %lld limit = %d printed = %d chunk %d\n",
		__func__, *pos, limit, printed, chunk);
	(*pos) += chunk; /* increase my position counter */
	chunk = CHUNK_PROC;

	if (*pos >= limit) { /* are we done */
		mutex_unlock(&proc_touch_lock);
		return NULL;
	} else if ((limit - *pos) < CHUNK_PROC) {
		chunk = limit - *pos;
	}

	memset(buf_chunk, 0, CHUNK_PROC);
	memcpy(buf_chunk, &driver_test_buff[(int)*pos], chunk);
	mutex_unlock(&proc_touch_lock);
	return buf_chunk;
}

/*
 * This function is called when there are no more data to print  the stream
 * need to be terminated or when PAGE_SIZE data were already written into the
 * sequential file
 * @param s pointer to the sequential file where to print the data
 * @param v pointer returned by fts_seq_next
 */
static void fts_seq_stop(struct seq_file *s, void *v)
{

	TS_LOG_INFO("%s in stop\n", __func__);

	if (v) {
		TS_LOG_INFO("%s return\n", __func__);
		return;
	}
	TS_LOG_INFO("%s v is null\n", __func__);
	mutex_lock(&proc_touch_lock);
	limit = 0;
	chunk = 0;
	printed = 0;

	TS_LOG_INFO("%s freeing and clearing driver_test_buff\n", __func__);
	kfree(driver_test_buff);
	driver_test_buff = NULL;
	mutex_unlock(&proc_touch_lock);
}

/*
 * Struct where define and specify the functions which implements the flow for
 * writing on a sequential file
 */
static const struct seq_operations fts_seq_ops = {
	.start = fts_seq_start,
	.next = fts_seq_next,
	.stop = fts_seq_stop,
	.show = fts_seq_show
};

/*
 * This function open a sequential file
 * @param inode Inode in the file system that was called and triggered this
 * function
 * @param file file associated to the file node
 * @return error code, 0 if success
 */
static int fts_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &fts_seq_ops);
};

/*
 * Receive the OP code and the inputs from shell when the file node is called,
 * parse it and then execute the corresponding function
 * echo cmd+parameters > /proc/fts/driver_test to execute the select command
 * cat /proc/fts/driver_test	to obtain the result into the shell
 * the string returned in the shell is made up as follow:
 * start byte
 * the answer content and format strictly depend on the cmd executed. In
 * general can be: an HEX string or a byte array (e.g in case of 0xF- commands)
 * end byte
 */
static ssize_t fts_driver_test_write(struct file *file,
	const char __user *buf, size_t count, loff_t *pos)
{
	int number_param = 0;
	char *p = NULL;
	int res = OK;
	int j;
	int index = 0;
	int size = DRIVER_TEST_INITIAL_SIZE;
	int temp;
	u16 byte_to_read = 0;
	u32 file_size = 0;
	u8 *read_data = NULL;
	u64 addr = 0;
	LimitFile lim;
	u8 cmd[DEIVER_COUNT_SIZE] = {0}; /* worst case needs count bytes */
	u32 func_to_test[FUNC_TEST_LEN] = {0};
	char pbuf[DEIVER_COUNT_SIZE] = {0};

	TS_LOG_INFO("%s +++\n", __func__);
	mutex_lock(&proc_touch_lock);
	if (count > DEIVER_COUNT_SIZE) {
		TS_LOG_ERR("%s count size max 800 error\n", __func__);
		res = ERROR_ALLOC;
		goto end;
	}
	mess.dummy = 0;
	mess.action = 0;
	mess.msg_size = 0;

	if (!buf) {
		TS_LOG_ERR("%s buf null\n", __func__);
		res = ERROR_ALLOC;
		goto end;
	}
	if (access_ok(VERIFY_READ, buf, count) < OK) {
		res = ERROR_ALLOC;
		goto end;
	}
	if (copy_from_user(pbuf, buf, count) != 0) {
		res = ERROR_ALLOC;
		goto end;
	}
	p = pbuf;
	if ((count > (MESSAGE_MIN_HEADER_SIZE - 1)) && (p[0] ==
		MESSAGE_START_BYTE)) {
		TS_LOG_INFO("%s enter in byte mode only support scriptless\n",
			__func__);
		mess.msg_size = (p[1] << 8) | p[2];
		mess.counter = (p[3] << 8) | p[4];
		mess.action = (p[5] << 8) | p[6];
		TS_LOG_INFO("%s: size = %d, counter_id = %d, action = %04X\n",
			 __func__, mess.msg_size, mess.counter, mess.action);
		size = MESSAGE_MIN_HEADER_SIZE + 2; /* +2 error code */
		if ((count < mess.msg_size) || (p[count - 2] !=
			MESSAGE_END_BYTE)) {
			TS_LOG_ERR("%s msg_size = %d != %d, last_byte = %02X != %02X error %08X\n",
				__func__, mess.msg_size, (int)count,
				p[count - 1], MESSAGE_END_BYTE,
				ERROR_OP_NOT_ALLOW);
			res = ERROR_OP_NOT_ALLOW;
			goto end;
		}
		/* +1 because put the internal op code */
		number_param = mess.msg_size - MESSAGE_MIN_HEADER_SIZE + 1;

		switch (mess.action) {
		case ACTION_READ:
			cmd[0] = func_to_test[0] = CMD_READ_BYTE;
			break;
		case ACTION_WRITE:
			cmd[0] = func_to_test[0] = CMD_WRITE_BYTE;
			break;
		case ACTION_WRITE_READ:
			cmd[0] = func_to_test[0] = CMD_WRITE_READ_BYTE;
			break;
		case ACTION_GET_VERSION:
			cmd[0] = func_to_test[0] = CMD_VERSION_BYTE;
			break;
		case ACTION_WRITE_THEN_WRITE_READ:
			cmd[0] = func_to_test[0] =
				CMD_WRITE_THEN_WRITE_READ_BYTE;
			break;
		case ACTION_WRITE_U8UX:
			cmd[0] = func_to_test[0] = CMD_WRITE_U8UX_BYTE;
			break;
		case ACTION_WRITE_READ_U8UX:
			cmd[0] = func_to_test[0] =
				CMD_WRITE_READ_U8UX_BYTE;
			break;
		case ACTION_WRITE_U8UX_THEN_WRITE_U8UX:
			cmd[0] = func_to_test[0] =
				CMD_WRITE_U8UX_THEN_WRITE_U8UX_BYTE;
			break;
		case ACTION_WRITE_U8UX_THEN_WRITE_READ_U8UX:
			cmd[0] = func_to_test[0] =
				CMD_WRITE_U8UX_THEN_WRITE_READ_BYTE;
			break;
		case ACTION_GET_FW:
			cmd[0] = func_to_test[0] =
				CMD_GET_FW_FILE_BYTE;
			break;
		case ACTION_GET_LIMIT:
			cmd[0] = func_to_test[0] =
				CMD_GET_LIMITS_FILE_BYTE;
			break;
		default:
			TS_LOG_ERR("%s invalid action = %d error %08X\n",
				__func__, mess.action,
				ERROR_OP_NOT_ALLOW);
			res = ERROR_OP_NOT_ALLOW;
			goto end;
		}
		/* -1 because i need to exclude the cmd[0] */
		if (((number_param - 1) > 0) && (number_param < count))
			memcpy(&cmd[1], &p[CMD_BUF_SIZE], number_param - 1);
	}

	if (number_param < 1) {
		TS_LOG_ERR("%s echo cmd_code args > stm_fts_cmd\n", __func__);
		res = ERROR_OP_NOT_ALLOW;
		goto end;
	}
	TS_LOG_INFO("%s number of parameters = %d\n", __func__, number_param);
	switch (func_to_test[0]) {
	case CMD_VERSION_BYTE:
		TS_LOG_INFO("%s: get version byte\n", __func__);
		byte_to_read = 2;
		mess.dummy = 0;
		read_data = kzalloc((byte_to_read * sizeof(u8)), GFP_KERNEL);
		if (read_data == NULL) {
			res = ERROR_ALLOC;
			TS_LOG_ERR("%s: read_data kzalloc error %08X\n",
				__func__, res);
			break;
		}
		size += byte_to_read;

		/*
		 * get driver version , high 8 bit is readData[0],
		 * and high 16 bit is readData[1]
		 */
		read_data[0] = (u8)(FTS_TS_DRV_VER >> 24);
		read_data[1] = (u8)(FTS_TS_DRV_VER >> 16);
		res = OK;
		TS_LOG_INFO("%s version = %02X%02X\n", __func__,
			read_data[0], read_data[1]);
		break;
	case CMD_WRITE_READ_BYTE:
		if (number_param < CMD_WRITE_READ_PARAM) {
			TS_LOG_INFO("%s wrong number of parameters\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		temp = number_param - CMD_PARAM_TEMP;
		if (cmd[number_param - 1] == 0)
			mess.dummy = 0;
		else
			mess.dummy = 1;

		u8_to_u16_be(&cmd[number_param - 3], &byte_to_read);
		TS_LOG_INFO("%s bytes to read = %d\n", __func__,
			byte_to_read + mess.dummy);
		read_data = kzalloc((byte_to_read +
			mess.dummy) * sizeof(u8), GFP_KERNEL);
		if (read_data == NULL) {
			res = ERROR_ALLOC;
			TS_LOG_INFO("%s: read_data kzalloc error %08X\n",
				__func__, res);
		} else {
			res = fts_writeRead(&cmd[1], temp, read_data,
				byte_to_read + mess.dummy);
			size += (byte_to_read * sizeof(u8));
		}
		break;
	case CMD_WRITE_BYTE:
		if (number_param >= CMD_WRITE_PARAM) {
			temp = number_param - 1;
			res = fts_write(&cmd[1], temp);
		} else {
			TS_LOG_ERR("%s wrong number of parameters\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
		}
		break;
	case CMD_READ_BYTE:
		if (number_param < CMD_READ_PARAM) {
			TS_LOG_ERR("%s wrong number of parameters\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		if ((number_param == CMD_READ_PARAM) ||
			((number_param == CMD_PARAM_TEMP) &&
			(cmd[number_param - 1] == 0)))
			mess.dummy = 0;
		else
			mess.dummy = 1;
		u8_to_u16_be(&cmd[1], &byte_to_read);
		read_data = kzalloc((byte_to_read +
			mess.dummy) * sizeof(u8), GFP_KERNEL);
		if (read_data == NULL) {
			res = ERROR_ALLOC;
			TS_LOG_ERR("%s: read_data kzalloc error %08X\n",
				__func__, res);
		} else {
			res = fts_read(read_data, byte_to_read + mess.dummy);
			size += (byte_to_read * sizeof(u8));
		}
		break;
	case CMD_WRITE_THEN_WRITE_READ_BYTE:
		if (number_param < CMD_WRITE_THEN_WRITE_READ_PARAM) {
			TS_LOG_ERR("%s wrong number of parameters\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		u8_to_u16_be(&cmd[number_param - CMD_WRITE_PARAM],
			&byte_to_read);
		read_data = kzalloc((byte_to_read * sizeof(u8)), GFP_KERNEL);
		if (read_data == NULL) {
			res = ERROR_ALLOC;
			TS_LOG_ERR("%s: kzalloc error %08X\n", __func__, res);
			break;
		}

		/*
		 * bus write cmd cmd[3] is write cmd
		 * cmd[1] is write cmd length bus read cmd
		 * cmd[3 +(int)cmd[1]] is read cmd
		 * cmd[2] is read cmd length
		 */
		res = fts_write_then_write_read(
			&cmd[3], cmd[1], &cmd[3 + (int)cmd[1]], cmd[2],
			read_data, byte_to_read);
		if (res < OK)
			TS_LOG_ERR("%s write fail\n", __func__);
		size += (byte_to_read * sizeof(u8));
		break;
	case CMD_WRITE_U8UX_BYTE:

		/*
		 * need to pass: cmd addrSize addr[0]  addr[addrSize-1]
		 * data[0] data[1]
		 * cmd[2] is the length of cmd[3] u8 tu u64
		 */
		if (number_param < CMD_WRITE_U8UX_PARAM) {
			TS_LOG_ERR("%s wrong number of parameters\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		if (cmd[2] > sizeof(u64)) {
			TS_LOG_ERR("%s wrong address size\n", __func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		u8_to_u64_be(&cmd[3], &addr, cmd[2]);
		TS_LOG_INFO("%s addr = %016llX %ld\n", __func__, addr,
			(long int)addr);

		/*
		 * cmd[1] byte containing the op code to write
		 * cmd[2] address size in byte
		 * &cmd[3 +cmd[2]] pointer of a byte
		 * array which contain the bytes
		 * to write (numberParam -cmd[2]-3 size of data
		 */
		res = fts_writeU8UX(cmd[1], cmd[2], addr, &cmd[3 + cmd[2]],
			(number_param - cmd[2] - 3));
		break;
	case CMD_WRITE_READ_U8UX_BYTE:

		/*
		 * need to pass: cmd addrSize addr[0] addr[addrSize-1]
		 * byteToRead1 byteToRead0 hasDummyByte
		 * cmd[2] is the length of cmd[3] u8 tu u64
		 * cmd[2] is the length of cmd[numberParam-3] u8 tu u64
		 */
		if (number_param < CMD_WRITE_READ_PARAM_U8UX) {
			TS_LOG_ERR("%s wrong number of parameters\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		if (cmd[2] > sizeof(u64)) {
			TS_LOG_ERR("%s wrong address size\n", __func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		u8_to_u64_be(&cmd[3], &addr, cmd[2]);
		u8_to_u16_be(&cmd[number_param - 3], &byte_to_read);
		read_data = kzalloc((byte_to_read * sizeof(u8)), GFP_KERNEL);
		if (read_data == NULL) {
			res = ERROR_ALLOC;
			TS_LOG_ERR("%s allocate error %08X\n", __func__, res);
		} else {
			TS_LOG_INFO("%s addr = %016llX byte to read = %d\n",
				__func__, addr, byte_to_read);
			res = fts_writeReadU8UX(cmd[1], cmd[2], addr,
				read_data, byte_to_read,
				cmd[number_param - 1]);
			size += (byte_to_read * sizeof(u8));
		}
		break;
	case CMD_WRITE_U8UX_THEN_WRITE_U8UX_BYTE:
		if (number_param < CMD_WRITE_READ_PARAM_U8UX) {
			TS_LOG_ERR("%s wrong number of parameters\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		/* cmd[2] + cmd[4] is the length of cmd[5] u8 tu u64 */
		if ((cmd[2] + cmd[4]) <= sizeof(u64)) {
			u8_to_u64_be(&cmd[5], &addr, cmd[2] + cmd[4]);
			TS_LOG_INFO("%s addr = %016llX %lld\n",
				__func__, addr, addr);

			/*
			 * cmd[5 + cmd[2] + cmd[4]] pointer of a byte array
			 * which contain the bytes to write,
			 * numberParam - cmd[2] -cmd[4] - 5 size of data
			 */
			res = fts_chunk_write(cmd[1], cmd[2], cmd[3], cmd[4],
				addr, &cmd[5 + cmd[2] + cmd[4]],
				(number_param - cmd[2] - cmd[4] - 5));
		} else {
			TS_LOG_ERR("%s wrong address size\n", __func__);
			res = ERROR_OP_NOT_ALLOW;
		}
		break;
	case CMD_WRITE_U8UX_THEN_WRITE_READ_BYTE:

		/*
		 * need to pass: cmd1 addrSize1 cmd2 addrSize2 addr[0]
		 * addr[addrSize1+addrSize2-1]  byteToRead1 byteToRead0
		 * hasDummybyte
		 */
		if (number_param < CMD_WRITE_THEN_READ_PARAM) {
			TS_LOG_ERR("%s wrong number of parameters\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		if ((cmd[2] + cmd[4]) > sizeof(u64)) {
			TS_LOG_ERR("%s wrong total address size\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		u8_to_u64_be(&cmd[5], &addr, cmd[2] + cmd[4]);
		TS_LOG_INFO("%s: cmd[5] = %02X, addr = %016llX\n",
			__func__, cmd[5], addr);
		u8_to_u16_be(&cmd[number_param - 3], &byte_to_read);
		read_data = kzalloc((byte_to_read * sizeof(u8)), GFP_KERNEL);
		if (read_data == NULL) {
			res = ERROR_ALLOC;
			TS_LOG_ERR("%s: read_data kzalloc error %08X\n",
				__func__, res);
		} else {
			res = fts_chunk_write_read(cmd[1], cmd[2], cmd[3],
				cmd[4], addr, read_data, byte_to_read,
				cmd[number_param - 1]);
			size += (byte_to_read * sizeof(u8));
		}
		break;
	case CMD_GET_LIMITS_FILE_BYTE:
		/* need to pass: byteToRead1 byteToRead0 */
		if (number_param < GET_FW_FILE_PARAM) {
			TS_LOG_ERR("%s wrong number of parameters\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		lim.data = NULL;
		lim.size = 0;
		u8_to_u16_be(&cmd[1], &byte_to_read);
		/* number of words */
		addr = ((u64)byte_to_read) * BYTE_SIZE;
		res = getLimitsFile(LIMITS_FILE, &lim);

		read_data = lim.data;
		file_size = lim.size;
		if (file_size > addr) {
			TS_LOG_ERR("%s lim.size expected = %d, real = %d\n",
				__func__, byte_to_read, file_size);
			res = ERROR_OP_NOT_ALLOW;
		}
		size += (addr * sizeof(u8));
		break;
	case CMD_GET_FW_FILE_BYTE:
		/* need to pass: byteToRead1 byteToRead0 */
		if (number_param != GET_FW_FILE_PARAM) {
			TS_LOG_ERR("%s wrong number of parameters\n",
				__func__);
			res = ERROR_OP_NOT_ALLOW;
			break;
		}
		u8_to_u16_be(&cmd[1], &byte_to_read);
		/* number of words */
		addr = ((u64)byte_to_read) * BYTE_SIZE;
		res = getFWdata(PATH_FILE_FW, &read_data, &file_size);
		if (file_size > addr) {
			TS_LOG_ERR("%s fw size expected = %d, real = %d\n",
				 __func__, byte_to_read, file_size);
			res = ERROR_OP_NOT_ALLOW;
		}
		size += (addr * sizeof(u8));
		break;
	default:
		TS_LOG_ERR("%s command id not valid\n", __func__);
		res = ERROR_OP_NOT_ALLOW;
		break;
	}
end:
	TS_LOG_INFO("%s free the buffer with the previous result\n", __func__);
	kfree(driver_test_buff);
	driver_test_buff = NULL;

	/*
	 * need to code each byte as HEX string,
	 * start byte is just one, the extra byte of end byte taken by
	 * size  initial value is 6 DRIVER_TEST_INITIAL_SIZE
	 */
	size *= 2;
	size -= 1;
	TS_LOG_INFO("%s size = %d\n", __func__, size);
	driver_test_buff = kzalloc(size, GFP_KERNEL);
	if (driver_test_buff == NULL) {
		TS_LOG_ERR("%s kzalloc driver_test_buff error %08X\n",
			__func__, ERROR_ALLOC);
		goto error;
	}
	TS_LOG_INFO("%s finish to allocate memory\n", __func__);
	driver_test_buff[index++] = MESSAGE_START_BYTE;
	if ((func_to_test[0] == CMD_GET_LIMITS_FILE_BYTE) ||
		(func_to_test[0] == CMD_GET_FW_FILE_BYTE))
		/* Take File_size high eight bits and low eight-bit values */
		snprintf(&driver_test_buff[index], BUFF_LENGTH, "%02X%02X",
			(((file_size + 3) / 4) & 0xFF00) >> 8,
			((file_size + 3) / 4) & 0x00FF);
	else
		snprintf(&driver_test_buff[index], BUFF_LENGTH,
			"%02X%02X", (size & 0xFF00) >> 8, size & 0xFF);

	index += 4; /* the first four buf is used as above */
	index += snprintf(&driver_test_buff[index], BUFF_LENGTH,
		"%04X", (u16)mess.counter);
	index += snprintf(&driver_test_buff[index], BUFF_LENGTH,
		"%04X", (u16)mess.action);
	index += snprintf(&driver_test_buff[index], BUFF_LENGTH,
		"%02X%02X", (res & 0xFF00) >> 8, res & 0xFF);
	if ((res >= OK) && (size - index - BUFF_END_LEN > byte_to_read) &&
		(size - index - BUFF_END_LEN > file_size)) {
		switch (func_to_test[0]) {
		case CMD_VERSION_BYTE:
		case CMD_READ_BYTE:
		case CMD_WRITE_READ_BYTE:
		case CMD_WRITE_THEN_WRITE_READ_BYTE:
		case CMD_WRITE_READ_U8UX_BYTE:
		case CMD_WRITE_U8UX_THEN_WRITE_READ_BYTE:
			for (j = mess.dummy; j < (byte_to_read + mess.dummy);
				j++)
				index += snprintf(&driver_test_buff[index],
					BUFF_LEN, "%02X", (u8)read_data[j]);
			break;
		case CMD_GET_LIMITS_FILE_BYTE:
		case CMD_GET_FW_FILE_BYTE:
			for (j = 0; j < file_size; j++)
				index += snprintf(&driver_test_buff[index],
					BUFF_LEN, "%02X", (u8)read_data[j]);
			for (; j < addr; j++)
				/* pad memory with 0x00 */
				index += snprintf(&driver_test_buff[index],
					BUFF_LEN, "%02X", 0);
			break;
		default:
			TS_LOG_ERR("%s default", __func__);
			break;
		}
	}
	driver_test_buff[index++] = MESSAGE_END_BYTE;
	driver_test_buff[index] = '\n';
	limit = size;
	printed = 0;
error:
	TS_LOG_INFO("%s ---\n", __func__);
	number_param = 0;
	kfree(read_data);
	mutex_unlock(&proc_touch_lock);

	return count;
}

/*
 * file_operations struct which define the functions for the canonical
 * operation on a device file node (open. read, write etc.)
 */
static const struct file_operations fts_driver_test_ops = {
	.open = fts_open,
	.read = seq_read,
	.write = fts_driver_test_write,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * This function is called in the probe to initialize and create the directory
 * proc/fts and the driver test file node DRIVER_DEBUG_FILE_NODE into the /proc
 * file system
 * @return OK if success or an error code which specify the type of error
 */
int fts_proc_init(void)
{
	struct proc_dir_entry *entry = NULL;
	int retval;

	fts_dir = proc_mkdir_data("fts", 0660, NULL, NULL);
	if (fts_dir == NULL) { /* directory creation failed */
		retval = -ENOMEM;
		goto out;
	}
	entry = proc_create(DRIVER_DEBUG_FILE_NODE, 0660, fts_dir,
		&fts_driver_test_ops);
	if (entry) {
		TS_LOG_INFO("%s: proc entry created\n", __func__);
	} else {
		TS_LOG_ERR("%s: error creating proc entry\n", __func__);
		retval = -ENOMEM;
		goto bad_file;
	}
	return OK;
bad_file:
	remove_proc_entry("fts", NULL);
out:
	return retval;
}

/*
 * Delete and Clean from the file system, all the references to the driver test
 * file node
 * @return OK
 */
int fts_proc_remove(void)
{
	remove_proc_entry(DRIVER_DEBUG_FILE_NODE, fts_dir);
	remove_proc_entry("fts", NULL);
	return OK;
}

