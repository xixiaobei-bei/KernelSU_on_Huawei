/*
  *
  **************************************************************************
  **                        STMicroelectronics				**
  **************************************************************************
  **                        marco.cali@st.com				**
  **************************************************************************
  *                                                                        *
  *                     I2C/SPI Communication				  *
  *                                                                        *
  **************************************************************************
  **************************************************************************
  *
  */

/*!
  * \file ftsIO.c
  * \brief Contains all the functions which handle with the I2C/SPI
  *communication
  */


#include "ftsSoftware.h"

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <stdarg.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/of_gpio.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
static u16 I2CSAD;	/* /< slave address of the IC in the i2c bus */

static void *client;	/* /< bus client retrived by the OS and
				  * used to execute the bus transfers */

#include "ftsCore.h"
#include "ftsError.h"
#include "ftsHardware.h"
#include "ftsIO.h"

#define ADDR_SIZE 8

/**
  * Initialize the static client variable of the fts_lib library in order
  * to allow any i2c/spi transaction in the driver (Must be called in the probe)
  * @param clt pointer to i2c_client or spi_device struct which identify the bus
  * slave device
  * @return OK
  */
int openChannel(void *clt)
{
	client = clt;
	if(clt){
		I2CSAD = ((struct i2c_client *)clt)->addr;
		TS_LOG_INFO("%s openChannel: SAD: %02X\n", __func__, I2CSAD);
		return OK;
	}

	return ERROR_OP_NOT_ALLOW;
}

/**
  * Change the I2C slave address which will be used during the transaction
  * (For Debug Only)
  * @param sad new slave address id
  * @return OK
  */
int changeSAD(u8 sad)
{
	I2CSAD = sad;
	return OK;
}

/**
  * Retrieve the pointer to the device struct of the IC
  * @return a the device struct pointer if client was previously set
  * or NULL in all the other cases
  */
struct device *getDev(void)
{
	struct i2c_client * temp = getClient();
	if (temp != NULL){
		return &(temp->dev);
	} else {
		return NULL;
	}
}


/**
  * Retrieve the pointer of the i2c_client struct representing the IC as i2c
  *slave
  * @return client if it was previously set or NULL in all the other cases
  */
struct i2c_client *getClient()
{
	if (client != NULL)
		return (struct i2c_client *)client;
	else
		return NULL;
}




/****************** New I2C API *********************/

/**
  * Perform a direct bus read
  * @param outBuf pointer of a byte array which should contain the byte read
  *from the IC
  * @param byteToRead number of bytes to read
  * @return OK if success or an error code which specify the type of error
  */
int fts_read(u8 *outBuf, int byteToRead)
{
	int ret = -1;
	int retry = 0;
	struct i2c_msg I2CMsg[1];

	if (!client || !I2CSAD ||!outBuf){
		TS_LOG_ERR( "%s: input error.\n", __func__);
		return ERROR_BUS_O;
	}

	I2CMsg[0].addr = (__u16)I2CSAD;
	I2CMsg[0].flags = (__u16)I2C_M_RD;
	I2CMsg[0].len = (__u16)byteToRead;
	I2CMsg[0].buf = (__u8 *)outBuf;

	while (retry < I2C_RETRY && ret < OK) {
		ret = i2c_transfer(((struct i2c_client *)client)->adapter, I2CMsg, 1);
		retry++;
		if (ret < OK)
			msleep(I2C_WAIT_BEFORE_RETRY);
		TS_LOG_ERR("%s fts_writeCmd: attempt %d\n", __func__, retry);
	}
	if (ret < 0) {
		TS_LOG_ERR( "%s: ERROR %08X\n", __func__, ERROR_BUS_R);
		return ERROR_BUS_R;
	}
	return OK;
}


/**
  * Perform a bus write followed by a bus read without a stop condition
  * @param cmd byte array containing the command to write
  * @param cmdLength size of cmd
  * @param outBuf pointer of a byte array which should contain the bytes read
  *from the IC
  * @param byteToRead number of bytes to read
  * @return OK if success or an error code which specify the type of error
  */
int fts_writeRead(u8 *cmd, int cmdLength, u8 *outBuf, int byteToRead)
{
	int ret = -1;
	int retry = 0;

	struct fts_ts_info *info = fts_get_info();
	if (!info|| !info->chip_data  ||  !info->chip_data->ts_platform_data
			 || !info->chip_data->ts_platform_data->bops
			 || !info->chip_data->ts_platform_data->bops->bus_read){
		TS_LOG_ERR( "%s: input error.\n", __func__);
		return ERROR_BUS_O;
	}
	while (retry < I2C_RETRY && ret < OK) {
		ret= info->chip_data->ts_platform_data->bops->bus_read(cmd,
			cmdLength, outBuf, byteToRead);
		retry++;
		if (ret < OK)
			msleep(I2C_WAIT_BEFORE_RETRY);
	}
	if (ret < 0) {
		TS_LOG_ERR( "%s: ERROR %08X\n", __func__, ERROR_BUS_WR);
		return ERROR_BUS_WR;
	}
	return OK;

}


/**
  * Perform a bus write
  * @param cmd byte array containing the command to write
  * @param cmdLength size of cmd
  * @return OK if success or an error code which specify the type of error
  */
int fts_write(u8 *cmd, int cmdLength)
{
	int ret = -1;
	int retry = 0;
	struct fts_ts_info *info = fts_get_info();

	if (!info|| !info->chip_data  ||  !info->chip_data->ts_platform_data
			 || !info->chip_data->ts_platform_data->bops
			 || !info->chip_data->ts_platform_data->bops->bus_write){
		TS_LOG_ERR( "%s: input error.\n", __func__);
		return ERROR_BUS_O;
	}

	while (retry < I2C_RETRY && ret < OK) {
		ret=info->chip_data->ts_platform_data->bops->bus_write(cmd, cmdLength);
		retry++;
		if (ret < OK)
			msleep(I2C_WAIT_BEFORE_RETRY);
		TS_LOG_DEBUG("%s fts_writeCmd: attempt %d\n", __func__, retry);
	}
	if (ret < 0) {
		TS_LOG_ERR( "%s: ERROR %08X\n", __func__, ERROR_BUS_W);
		return ERROR_BUS_W;
	}
	return OK;

}

/**
  * Write a FW command to the IC and check automatically the echo event
  * @param cmd byte array containing the command to send
  * @param cmdLength size of cmd
  * @return OK if success, or an error code which specify the type of error
  */
int fts_writeFwCmd(u8 *cmd, int cmdLength)
{
	struct fts_ts_info *info = fts_get_info();
	int retry = 0;
	int ret = 0;
	int ret2 = 0;

	if (!info|| !info->chip_data  ||  !info->chip_data->ts_platform_data
			 || !info->chip_data->ts_platform_data->bops
			 || !info->chip_data->ts_platform_data->bops->bus_write){
		TS_LOG_ERR( "%s: input error.\n", __func__);
		return ERROR_BUS_O;
	}

	do {
		ret = info->chip_data->ts_platform_data->bops->bus_write(cmd, cmdLength);
		if (ret >= 0)
			ret2 = checkEcho(cmd, cmdLength);

		if(ret < OK || ret2 < OK)
			mdelay(I2C_WAIT_BEFORE_RETRY);
		retry++;
	} while(retry<I2C_RETRY && (ret < OK || ret2 < OK));
	if (ret2 < OK) {
		TS_LOG_ERR("%s: checkEcho error\n", __func__);
		ret = ret2;
	}
	return ret;
}


/*
 * Perform two bus write and one bus read without any stop condition
 * In case of FTI this function is not supported and the same sequence
 * can be achieved calling fts_write followed by an fts_writeRead.
 * @param writeCmd1 byte array containing the first command to write
 * @param writeCmdLength size of writeCmd1
 * @param readCmd1 byte array containing the second command to write
 * @param readCmdLength size of readCmd1
 * @param outBuf pointer of a byte array which should contain the bytes read
 * from the IC
 * @param byteToRead number of bytes to read
 * @return OK if success or an error code which specify the type of error
 */
int fts_write_then_write_read(u8 *write_cmd, int write_cmd_length,
	u8 *read_cmd, int read_cmd_length, u8 *out_buf, int byte_to_read)
{
	struct fts_ts_info *info = fts_get_info();

	if ((!write_cmd) || (!read_cmd) || (!out_buf)) {
		TS_LOG_ERR("%s:cmd null\n", __func__);
		return ERROR_BUS_W;
	}
	if ((!info) || (!info->chip_data) ||
		(!info->chip_data->ts_platform_data) ||
		(!info->chip_data->ts_platform_data->bops) ||
		(!info->chip_data->ts_platform_data->bops->bus_read) ||
		(!info->chip_data->ts_platform_data->bops->bus_write)) {
		TS_LOG_ERR("%s: info null error\n", __func__);
		return ERROR_BUS_O;
	}
	if (info->chip_data->ts_platform_data->bops->bus_write(write_cmd,
		write_cmd_length))
		TS_LOG_ERR("%s:write fail\n", __func__);
	return info->chip_data->ts_platform_data->bops->bus_read(read_cmd,
			read_cmd_length, out_buf, byte_to_read);
}

/**
  * Perform a chunked write with one byte op code and 1 to 8 bytes address
  * @param cmd byte containing the op code to write
  * @param addrSize address size in byte
  * @param address the starting address
  * @param data pointer of a byte array which contain the bytes to write
  * @param dataSize size of data
  * @return OK if success or an error code which specify the type of error
  */
/* this function works only if the address is max 8 bytes */
int fts_writeU8UX(u8 cmd, AddrSize addrSize, u64 address, u8 *data, int
		  dataSize)
{
	u8 finalCmd[2 + addrSize + WRITE_CHUNK];
	int remaining = dataSize;
	int toWrite = 0, i = 0;

	if ((addrSize >sizeof(u64)) || !data) {
		TS_LOG_ERR("%s: address size bigger than max allowed %ld... ERROR %08X\n",
			 __func__, sizeof(u64), ERROR_OP_NOT_ALLOW);
		return ERROR_BUS_W;
	}
	while (remaining > 0) {
		if (remaining >= WRITE_CHUNK) {
			toWrite = WRITE_CHUNK;
			remaining -= WRITE_CHUNK;
		} else {
			toWrite = remaining;
			remaining = 0;
		}

		finalCmd[0] = cmd;
		TS_LOG_INFO( "%s: addrSize = %d\n", __func__,addrSize);
		for (i = 0; i < addrSize; i++) {
			finalCmd[i + 1] = (u8)((address >> ((addrSize - 1 - i) * 8)) & 0xFF);
			TS_LOG_ERR( "%s: cmd[%d] = %02X\n", __func__,i + 1, finalCmd[i + 1]);
		}

		memcpy(&finalCmd[addrSize + 1], data, toWrite);

		if (fts_write(finalCmd, 1 + addrSize + toWrite) < OK) {
			TS_LOG_ERR( "%s: ERROR %08X\n", __func__,ERROR_BUS_W);
			return ERROR_BUS_W;
		}

		address += toWrite;
		data += toWrite;
	}

	return OK;
}

/**
  * Perform a chunked write read with one byte op code and 1 to 8 bytes address
  * and dummy byte support.
  * @param cmd byte containing the op code to write
  * @param addrSize address size in byte
  * @param address the starting address
  * @param outBuf pointer of a byte array which contain the bytes to read
  * @param byteToRead number of bytes to read
  * @param hasDummyByte  if the first byte of each reading is dummy (must be
  * skipped)
  * set to 1, otherwise if it is valid set to 0 (or any other value)
  * @return OK if success or an error code which specify the type of error
  */
int fts_writeReadU8UX(u8 cmd, AddrSize addrSize, u64 address, u8 *outBuf, int
		      byteToRead, int hasDummyByte)
{
	u8 finalCmd[1 + addrSize];
	u8 buff[READ_CHUNK + 1] = {0};/* worst case has dummy byte */
	int remaining = byteToRead;
	int toRead = 0, i = 0;

	if ((addrSize > sizeof(u64)) || !outBuf) {
		TS_LOG_ERR("%s: address size bigger than max allowed %ld... ERROR %08X\n",
			 __func__, sizeof(u64), ERROR_OP_NOT_ALLOW);
		return ERROR_BUS_W;
	}

	while (remaining > 0) {
		if (remaining >= READ_CHUNK) {
			toRead = READ_CHUNK;
			remaining -= READ_CHUNK;
		} else {
			toRead = remaining;
			remaining = 0;
		}

		finalCmd[0] = cmd;
		for (i = 0; i < addrSize; i++)
			finalCmd[i + 1] = (u8)((address >> ((addrSize - 1 - i) *  8)) & 0xFF);

		if (hasDummyByte == 1) {
			if (fts_writeRead(finalCmd, 1 + addrSize, buff, toRead + 1) < OK) {
				TS_LOG_ERR("%s: read error... ERROR %08X\n",__func__, ERROR_BUS_WR);
				return ERROR_BUS_WR;
			}
			memcpy(outBuf, buff + 1, toRead);
		} else {
			if (fts_writeRead(finalCmd, 1 + addrSize, buff, toRead) < OK) {
				TS_LOG_ERR("%s: read error... ERROR %08X\n",__func__, ERROR_BUS_WR);
				return ERROR_BUS_WR;
			}
			memcpy(outBuf, buff, toRead);
		}

		address += toRead;
		outBuf += toRead;
	}

	return OK;
}

/*
 * Perform a chunked write followed by a second write with one byte op code
 * for each write and 1 to 8 bytes address (the sum of the 2 address size of
 * the two writes can not exceed 8 bytes)
 * @param cmd1 byte containing the op code of first write
 * @param addrSize1 address size in byte of first write
 * @param cmd2 byte containing the op code of second write
 * @param addrSize2 address size in byte of second write
 * @param address the starting address
 * @param data pointer of a byte array which contain the bytes to write
 * @param dataSize size of data
 * @return OK if success or an error code which specify the type of error
 * this function works only if the sum of two addresses in the two commands is
 * max 8 bytes
 */
int fts_chunk_write(u8 cmd_first, AddrSize addr_size_first, u8 cmd_second,
	AddrSize addr_size_second, u64 address, u8 *data, int data_size)
{
	int to_write;
	int i;
	int remaining = data_size;
	/*
	 * Dynamic application memory, addr_size_first addr_size_second
	 * Maximum Range 0 ~ 255
	 */
	u8 final_cmd_first[1 + addr_size_first];
	u8 final_cmd_second[1 + addr_size_second + WRITE_CHUNK];

	if (!data) {
		TS_LOG_ERR("%s data null\n", __func__);
		return ERROR_BUS_W;
	}
	while (remaining > 0) {
		if (remaining >= WRITE_CHUNK) {
			to_write = WRITE_CHUNK;
			remaining -= WRITE_CHUNK;
		} else {
			to_write = remaining;
			remaining = 0;
		}
		final_cmd_first[0] = cmd_first;
		for (i = 0; i < addr_size_first; i++)
			final_cmd_first[i + 1] = (u8)((address >>
				((addr_size_first + addr_size_second - 1 - i) *
				ADDR_SIZE)) & 0xFF);
		final_cmd_second[0] = cmd_second;
		for (i = addr_size_first; i < addr_size_first +
			addr_size_second; i++)
			final_cmd_second[i - addr_size_first + 1] =
				(u8)((address >> ((addr_size_first +
				addr_size_second - 1 - i) * ADDR_SIZE)) &
				0xFF);

		memcpy(&final_cmd_second[addr_size_second + 1], data,
			to_write);
		if (fts_write(final_cmd_first, 1 + addr_size_first) < OK) {
			TS_LOG_ERR("%s: first write error error %08X\n",
				__func__, ERROR_BUS_W);
			return ERROR_BUS_W;
		}

		if (fts_write(final_cmd_second, 1 + addr_size_second +
			to_write) < OK) {
			TS_LOG_ERR("%s: second write error %08X\n", __func__,
				ERROR_BUS_W);
			return ERROR_BUS_W;
		}

		address += to_write;
		data += to_write;
	}
	return OK;
}

/*
 * Perform a chunked write  followed by a write read with one byte op code
 * and 1 to 8 bytes address for each write and dummy byte support.
 * @param cmd1 byte containing the op code of first write
 * @param addrSize1 address size in byte of first write
 * @param cmd2 byte containing the op code of second write read
 * @param addrSize2 address size in byte of second write	read
 * @param address the starting address
 * @param outBuf pointer of a byte array which contain the bytes to read
 * @param byteToRead number of bytes to read
 * @param hasDummyByte  if the first byte of each reading is dummy (must be
 * skipped) set to 1,
 * otherwise if it is valid set to 0 (or any other value)
 * @return OK if success or an error code which specify the type of error
 * this function works only if the sum of two addresses in the two commands is
 * max 8 bytes
 */
int fts_chunk_write_read(u8 cmd_first, AddrSize addr_size_first,
	u8 cmd_second, AddrSize addr_size_second, u64 address,
	u8 *out_buf, int byte_to_read, int has_dummy_byte)
{
	int to_read;
	int i;
	/*
	 * Dynamic application memory, addr_size_first addr_size_second
	 * Maximum Range 0 ~ 255
	 */
	u8 final_cmd_first[1 + addr_size_first];
	u8 final_cmd_second[1 + addr_size_second];
	u8 buff[READ_CHUNK + 1] = {0}; /* worst case has dummy byte */
	int remaining = byte_to_read;

	if (!out_buf) {
		TS_LOG_ERR("%s out_buf null\n", __func__);
		return ERROR_BUS_W;
	}
	while (remaining > 0) {
		if (remaining >= READ_CHUNK) {
			to_read = READ_CHUNK;
			remaining -= READ_CHUNK;
		} else {
			to_read = remaining;
			remaining = 0;
		}

		final_cmd_first[0] = cmd_first;
		for (i = 0; i < addr_size_first; i++)
			final_cmd_first[i + 1] = (u8)((address >>
				((addr_size_first + addr_size_second - 1 - i) *
				ADDR_SIZE)) & 0xFF);

		final_cmd_second[0] = cmd_second;
		for (i = addr_size_first; i < addr_size_first +
			addr_size_second; i++)
			final_cmd_second[i - addr_size_first + 1] =
				(u8)((address >> ((addr_size_first +
				addr_size_second - 1 - i) * ADDR_SIZE)) &
				0xFF);

		if (fts_write(final_cmd_first, 1 + addr_size_first) < OK) {
			TS_LOG_ERR("%s: first write error %08X\n", __func__,
				ERROR_BUS_W);
			return ERROR_BUS_W;
		}

		if (has_dummy_byte == 1) {
			if (fts_writeRead(final_cmd_second,
				1 + addr_size_second, buff,
				to_read + 1) < OK) {
				TS_LOG_ERR("%s: read error %08X\n", __func__,
					ERROR_BUS_WR);
				return ERROR_BUS_WR;
			}
			memcpy(out_buf, buff + 1, to_read);
		} else {
			if (fts_writeRead(final_cmd_second,
				1 + addr_size_second, buff,
				to_read) < OK) {
				TS_LOG_ERR("%s: read error %08X\n", __func__,
					ERROR_BUS_WR);
				return ERROR_BUS_WR;
			}
			memcpy(out_buf, buff, to_read);
		}

		address += to_read;
		out_buf += to_read;
	}

	return OK;
}

